
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	int j, k;
	PROCESS* next;
	
	refresh(ready);

	/*	就绪队列不为空	*/
	if (ready[0] != -1){
		if (p_proc_ready->state == 0x0001){
			/*	state: wait	*/
			/*	ready queue默认整齐*/
			next = &proc_table[ready[0]];
			dequeue(ready);
			refresh(ready);
			p_proc_ready = next;
		}else if (p_proc_ready->state == 0x0002){
			/*	初始：直接开始; 可能用不到	*/
			dequeue(ready);
			refresh(ready);
		}else if (p_proc_ready->state == 0x0003){
			enqueue(p_proc_ready, ready);
			next =  &proc_table[ready[0]];
			dequeue(ready);
			refresh(ready);
			p_proc_ready = next;
		}else if (p_proc_ready->state == 0x0004){
			/*	finish	*/
			if (p_proc_ready->times > 0){
				p_proc_ready->state = 0x0002;
				refresh(ready);
				enqueue(p_proc_ready, ready);
			}else{
				p_proc_ready->state = 0x0001;
			}
		}else{
			/*	code	*/
		}
	}else{
		/*	code	*/
		if (p_proc_ready->type == 0x0003){
			/*	只剩下F进程且正在运行的F进程	*/
			/*	需要将ready再充满，且次数也需重置	*/
			/*	若有剩余次数但在睡眠的进程，则会优先等该进程结束睡眠，其间都是F进程空闲时间	*/

			for (int i=0; i < NR_TASKS; i++){
				if (proc_table[i].sleep_time != 0){
					enqueue(p_proc_ready, ready);
					refresh(ready);
					next =  &proc_table[ready[0]];
					dequeue(ready);
					refresh(ready);
					p_proc_ready = next;
					return;
				}
			}

			reset();

			for (int i=0; i < NR_TASKS; i++){
				ready[i] = i;
			}
			next =  &proc_table[ready[0]];
			dequeue(ready);
			refresh(ready);
			p_proc_ready = next;
		}
	}

	return;
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                           sys_get_delay
 *======================================================================*/
PUBLIC void sys_get_delay(int milli_sec)
{
	get_sleep(milli_sec);
}

/*======================================================================*
                           sys_print_str
 *======================================================================*/
PUBLIC void sys_print_str(char *str)
{
	refresh_screen();
	disp_str("sys_print_str call:");
	disp_str(str);
	disp_str("\n");
}

/*======================================================================*
                           PV operation system call
 *======================================================================*/
PUBLIC void sys_p_operation(SEMAPHORE *semaphore)
{
	semaphore->value--;

	if (semaphore == &read)
	{
		if (rc >= LIMIT_READ)
		{
			sleep(semaphore->queue);
		}
		return;
	}
	if (semaphore->value < 0)
	{
		sleep(semaphore->queue);
	}
}

PUBLIC void sys_v_operation(SEMAPHORE *semaphore)
{
	semaphore->value++;
	int i = 0;
	if (p_proc_ready->type == 0x0001 && semaphore == &read){
		p_proc_ready->state = 0x0004;
		i = 1;
	}
	if (p_proc_ready->type == 0x0002 && semaphore == &write){
		p_proc_ready->state = 0x0004;
		i = 1;
	}

	if (semaphore->value <= 0){
		if (getNum(semaphore->queue) != 0){
			wakeUp(semaphore->queue);
		}	
	}

	if (i == 1){
		/*	完成的进程放在唤醒进程后面合理	*/
		schedule();
	}
 
}

PUBLIC void sleep(int *queue)
{
	p_proc_ready->state = 0x0001;
	enqueue(p_proc_ready, queue);

	schedule();

	return;
}

PUBLIC void wakeUp(int *queue)
{
	/*	读者优先，优先唤醒被read堵塞住的进程	*/

	int i;
	for (i = 0; i < NR_TASKS; i++)
	{
		if (proc_table[queue[i]].times > 0 && proc_table[queue[i]].sleep_time == 0)
		{
			int ready_proc = dequeue(&queue[i]);
			refresh(queue);
			PROCESS *p_ready = &proc_table[ready_proc];
			p_ready->state = 0x0002;
			enqueue(p_ready, ready);

			return;
		}
	}
	return;
}

PUBLIC void get_sleep(int milli_sec)
{
	p_proc_ready->state = 0x0001;
	p_proc_ready->sleep_time = milli_sec;

	schedule();

	return;
}