
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK *p_task = task_table;
	PROCESS *p_proc = proc_table;
	char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++)
	{
		strcpy(p_proc->p_name, p_task->name); // name of the process
		p_proc->pid = i;					  // pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
			   sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
			   sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs = ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ds = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.es = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss = ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	/*	读者优先	*/
	first = 0;
	/*	读者限制	*/
	LIMIT_READ = 3;

	/*	初始化进程	*/
	proc_table[0].proc_time = 2 * 10000;
	proc_table[1].proc_time = 3 * 10000;
	proc_table[2].proc_time = 3 * 10000;
	proc_table[3].proc_time = 3 * 10000;
	proc_table[4].proc_time = 4 * 10000;
	proc_table[5].proc_time = 1 * 10000;

	proc_table[0].state = 0x0002;
	proc_table[1].state = 0x0002;
	proc_table[2].state = 0x0002;
	proc_table[3].state = 0x0002;
	proc_table[4].state = 0x0002;
	proc_table[5].state = 0x0002;

	proc_table[0].times = 1;
	proc_table[1].times = 1;
	proc_table[2].times = 1;
	proc_table[3].times = 1;
	proc_table[4].times = 1;
	proc_table[5].times = 1000;

	proc_table[0].type = proc_table[1].type = proc_table[2].type = 0x0001;
	proc_table[3].type = proc_table[4].type = 0x0002;
	proc_table[5].type = 0x0003;

	for (i = 0; i < NR_TASKS; i++)
	{
		proc_table[i].sleep_time = 0;
	}

	k_reenter = 0;
	ticks = 0;
	execution = 0;

	rc = 0;
	read.value = 1;
	write.value = 1;
	mutex.value = 1;
	for (i = 0; i < NR_TASKS; i++)
	{
		read.queue[i] = -1;
		write.queue[i] = -1;
		mutex.queue[i] = -1;
	}

	if (first == 0)
	{
		ready[0] = 0;
		ready[1] = 1;
		ready[2] = 2;
		ready[3] = 3;
		ready[4] = 4;
		ready[5] = 5;
	}
	if (first == 1)
	{
		ready[0] = 3;
		ready[1] = 4;
		ready[2] = 0;
		ready[3] = 1;
		ready[4] = 2;
		ready[5] = 5;
	}

	int initial_value = ready[0];
	p_proc_ready = &proc_table[initial_value];

	/*	第一个要求，用读者上限3个测试	*/
	// get_delay(1000);
	// print_str("Test!!!");

	/* 初始化 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
	out_byte(TIMER0, (u8)((TIMER_FREQ / HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
	enable_irq(CLOCK_IRQ);					   /* 让8259A可以接收时钟中断 */

	restart();

	while (1)
	{
	}
}

/*======================================================================*
                               Test
 *======================================================================*/
void TestA()
{
	while (1)
	{
		while (proc_table[0].state == 0x0001)
		{
		}
		p(&mutex);
		if (first == 1)
		{
			if (rc == 0)
				p(&write);
		}
		p(&read);
		p_proc_ready->state = 0x0003;
		if (first == 0)
		{
			if (rc == 0)
				p(&write);
		}
		rc = rc + 1;
		v(&mutex);
		p_proc_ready->times--;

		refresh_screen();
		disp_color_str("A start. ", 0x01);

		milli_delay(proc_table[0].proc_time);
		refresh_screen();
		disp_color_str("A finish. ", 0x01);

		rc = rc - 1;
		if (first == 0)
		{
			if (rc == 0)
				v(&write);
		}
		v(&read);
		if (first == 1)
		{
			if (rc == 0)
				v(&write);
		}
	}
}

void TestB()
{
	while (1)
	{
		while (proc_table[1].state == 0x0001)
		{
		}
		p(&mutex);
		if (first == 1)
		{
			if (rc == 0)
				p(&write);
		}
		p_proc_ready->state = 0x0003;
		if (first == 0)
		{
			if (rc == 0)
				p(&write);
		}
		rc = rc + 1;
		v(&mutex);
		p_proc_ready->times--;

		refresh_screen();
		disp_color_str("B start. ", 0x02);
		milli_delay(proc_table[1].proc_time);
		refresh_screen();
		disp_color_str("B finish. ", 0x02);

		rc = rc - 1;
		if (first == 0)
		{
			if (rc == 0)
				v(&write);
		}
		v(&read);
		if (first == 1)
		{
			if (rc == 0)
				v(&write);
		}
	}
}

void TestC()
{
	while (1)
	{
		while (proc_table[2].state == 0x0001)
		{
		}
		p(&mutex);
		if (first == 1)
		{
			if (rc == 0)
				p(&write);
		}
		p(&read);
		p_proc_ready->state = 0x0003;
		if (first == 0)
		{
			if (rc == 0)
				p(&write);
		}
		rc = rc + 1;
		v(&mutex);

		p_proc_ready->times--;

		refresh_screen();
		disp_color_str("C start. ", 0x04);
		milli_delay(proc_table[2].proc_time);
		refresh_screen();
		disp_color_str("C finish. ", 0x04);

		rc = rc - 1;
		if (first == 0)
		{
			if (rc == 0)
				v(&write);
		}
		v(&read);
		if (first == 1)
		{
			if (rc == 0)
				v(&write);
		}
	}
}

void TestD()
{
	while (1)
	{
		while (proc_table[3].state == 0x0001)
		{
		}

		p(&write);
		wc++;
		p_proc_ready->state = 0x0003;
		p_proc_ready->times--;

		refresh_screen();
		disp_color_str("D start. ", 0x05);
		milli_delay(proc_table[3].proc_time);
		refresh_screen();
		disp_color_str("D finish. ", 0x05);

		wc--;
		v(&write);
	}
}

void TestE()
{
	while (1)
	{
		while (proc_table[4].state == 0x0001)
		{
		}
		p(&write);
		wc++;

		p_proc_ready->state = 0x0003;

		p_proc_ready->times--;
		refresh_screen();
		disp_color_str("E start. ", 0x06);
		milli_delay(proc_table[4].proc_time);
		refresh_screen();
		disp_color_str("E finish. ", 0x06);

		wc--;
		v(&write);
	}
}

void TestF()
{
	while (1)
	{
		p_proc_ready->state = 0x0003;
		p_proc_ready->times--;
		int i;

		refresh_screen();
		// if (getNum(mutex.queue) == 0)
		// {

		// }
		disp_str("\n");
		if (wc == 1 && rc == 0)
		{
			refresh_screen();
			disp_str("writing. ");
		}
		else if (rc > 0 && wc == 0)
		{
			refresh_screen();
			if (rc == 1)
				disp_str("1 ");
			else if (rc == 2)
				disp_str("2 ");
			else if (rc == 3)
				disp_str("3 ");
			else
				disp_str("n ");
			disp_str("reading. ");
		}
		else
		{
			refresh_screen();
			disp_str("freetime. ");
		}

		milli_delay(proc_table[5].proc_time);
	}
}

/*======================================================================*
                               Operation
 *======================================================================*/

PUBLIC void enqueue(PROCESS *p, int *queue)
{
	refresh(queue);
	u32 pid = p->pid;
	int i;
	for (i = 0; i < NR_TASKS; i++)
	{
		if (queue[i] == -1)
		{
			queue[i] = pid;
			break;
		}
	}
}

/*	出队列直接出指针最初值	*/
PUBLIC int dequeue(int *queue)
{
	int i, j;
	j = *queue;
	*queue = -1;

	return j;
}

/*	刷新队列	*/
PUBLIC void refresh(int *queue)
{
	int i;
	for (i = 0; i < NR_TASKS - 1; i++)
	{
		if (queue[i] == -1 && queue[i + 1] != -1)
		{
			queue[i] = queue[i + 1];
			queue[i + 1] = -1;
		}
	}
}

/*	重置次数, 针对原始proc_table队列，其余队列不需要 */
PUBLIC void reset()
{
	for (int i = 0; i < NR_TASKS; i++)
	{
		if (i == 0 || i == 1 || i == 2)
		{
			proc_table[i].times = 1;
			proc_table[i].state = 0x0002;
		}
		else if (i == 3 || i == 4)
		{
			proc_table[i].times = 1;
			proc_table[i].state = 0x0002;
		}
		else
		{
			proc_table[i].times = 1000;
		}
	}
	return;
}

PUBLIC void refresh_screen()
{
	if (disp_pos > 80 * 23 * 2)
	{
		int i;
		disp_pos = 0;
		for (i = 0; i < 80 * 25; i++)
		{
			disp_str(" ");
		}
		disp_pos = 0;
	}
	return;
}

PUBLIC int getNum(int *queue)
{
	int j = 0;
	for (int i = 0; i < NR_TASKS; i++)
	{
		if (queue[i] != -1)
		{
			j++;
		}
	}
	return j;
}

PUBLIC int checkReady()
{
	int j = 0;
	for (int i = 0; i < NR_TASKS; i++)
	{
		if (ready[i] != -1)
		{
			if (proc_table[ready[i]].state != 0x0002)
			{
				j++;
			}
		}
	}
	return j;
}