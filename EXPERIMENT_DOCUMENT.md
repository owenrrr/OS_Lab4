### 操作系统第四次实验说明文档



```
要求一：添加系统调用，其接受int 型参数milli_seconds ，调用此方法进程会在milli_seconds毫秒内不被分配时间片
```

> syscall.asm

```nasm
get_delay:
	mov eax, _NR_get_delay		; 对应1
	mov esi, esp
	mov ebx, [esi + 4]
	int	INT_VECTOR_SYS_CALL
	ret
```

> kernel.asm

```nasm
sys_call:
        call    save
        sti
		push	ebx
        call    [sys_call_table + eax * 4]
        mov     [esi + EAXREG - P_STACKBASE], eax
		pop		ebx
        cli
        ret
```

> global.c

```c
PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_get_delay, sys_print_str, sys_p_operation, sys_v_operation};	/*	添加系统调用函数，根据NR_SYS_CALL判断调用函数	*/
```

> proc.c

```c
PUBLIC void sys_get_delay(int milli_sec)
{
	get_sleep(milli_sec);
}

PUBLIC void get_sleep(int milli_sec)
{
    /*	设置当前进程睡眠，并立即调用schedule	*/
	p_proc_ready->state = 0x0001;
	enqueue(p_proc_ready, wait_queue);
	p_proc_ready->sleep_time = milli_sec;

	schedule();

	return;
}
```

> clock.c

```c
PUBLIC void clock_handler(int irq)
{
	ticks++;
	p_proc_ready->ticks--;

	int i;
    /*	将睡眠进程睡眠时间减去	*/
	for (i = 0; i < NR_TASKS ; i++){
		if (wait_queue[i] != -1){
			if (proc_table[wait_queue[i]].sleep_time > 0){
				proc_table[wait_queue[i]].sleep_time--;
			}
		}
	}

	schedule();
}
```

- **后面再将wakeUp函数中唤醒的进程作sleep_time判断，即可实现睡眠进程挂起的效果**



```
要求二：• 添加系统调用打印字符串，接受char* 型参数str
```

> syscall.asm

```nasm
print_str:
	mov eax, _NR_print_str	; 对应2
	mov	esi, esp
	mov ebx, [esi + 4]		; char* str : 32bit
	int INT_VECTOR_SYS_CALL
	ret
```

> kernel.asm

```nasm
sys_call:
        call    save
        sti
		push	ebx
        call    [sys_call_table + eax * 4]
        mov     [esi + EAXREG - P_STACKBASE], eax
		pop		ebx
        cli
        ret
```

> global.c

```c
PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_get_delay, sys_print_str, sys_p_operation, sys_v_operation};	/*	添加系统调用函数，根据NR_SYS_CALL判断调用函数	*/
```

> proc.c

```c
PUBLIC void sys_print_str(char *str)
{
	refresh_screen();			/*	当前超出屏幕范围则打印 80 * 25空格并将disp_pos置为0	*/
	disp_str("sys_print_str call:");
	disp_str(str);
	disp_str("\n");
}
```



```
要求三：添加两个系统调用执行信号量PV 操作，在此基础上模拟读者写者问题。
```

**设计出一个就绪队列，一个等待队列；就绪队列会自动随时间片轮转而执行；若是遇到睡眠或进程运行完则将其丢到等待队列中，待条件允许情况下再将其放到就绪队列**

```c
int	ready_queue[NR_TASKS];
int wait_queue[NR_TASKS];
```

**进程饿死：使用队列和次数限制进程运行，在一个周期内该进程执行次数 >= LIMIT则该进程放到等待队列中直到周期结束**

> main.c

```c
PUBLIC int kernel_main()
{
	/*	初始化	*/
}

/*	reader	*/
void TestA()
{
	while (1)
	{
		while (proc_table[0].times <= 0)
		{
		}
		p(&mutex);
		p(&read);
		if (rc == 0)
			p(&write);
		rc = rc + 1;
		v(&mutex);

		refresh_screen();
		disp_color_str("A start. ", 0x01);

		/*	设为on_processing状态，能被允许的且为ready状态的进程抢占 */
		p_proc_ready->state = 0x0003;
		p_proc_ready->times--;
		milli_delay(proc_table[0].proc_time);
		refresh_screen();
		disp_color_str("A finish. ", 0x01);

		/*	将自己丢到wait_queue中	*/
		p_proc_ready->state = 0x0001;
		enqueue(p_proc_ready, wait_queue);

		rc = rc - 1;
		if (rc == 0)
			v(&write);
		v(&read);
	}
}

/*	writer	*/
void TestD()
{
	while (1)
	{
		while (proc_table[3].times <= 0)
		{
		}
		/*	第一个要求	*/
		// get_delay(10000);
		p(&write);

		if (wc == 0)
			wc = 1;

		refresh_screen();
		disp_color_str("D start. ", 0x05);
		p_proc_ready->state = 0x0003;
		p_proc_ready->times--;
		milli_delay(proc_table[3].proc_time);
		refresh_screen();
		disp_color_str("D finish. ", 0x05);

		/*	将自己进程丢到wait_queue中	*/
		p_proc_ready->state = 0x0001;
		enqueue(p_proc_ready, wait_queue);

		if (wc == 1)
			wc = 0;

		v(&write);
	}
}

/*	打印进程	*/
void TestF()
{
	while (1)
	{
		while (proc_table[5].times <= 0)
		{
		}

		int i;

		refresh_screen();
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
		p_proc_ready->state = 0x0003;
		p_proc_ready->times--;
		milli_delay(proc_table[5].proc_time);
		proc_table[5].state = 0x0002;
		enqueue(&proc_table[5], ready_queue);
	}
}

/*	进队列	*/
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

/*	重置次数，注意当有进程在睡眠但是该进程times == 0，则会等该进程运行完	*/
PUBLIC void reset(int *queue)
{
	int i;
	refresh(queue);

	/*	0:S空格 ;1:SPACE; 2:null; 3:=SSPACE	*/

	for (i = 0; i < NR_TASKS; i++)
	{
		if (queue[i] != -1)
		{
			if (queue[i] == 3 || queue[i] == 4)
			{
				proc_table[queue[i]].times = 1;
			}
			else if (queue[i] == 5)
			{
				proc_table[queue[i]].times = 1000;
			}
			else
			{
				proc_table[queue[i]].times = 1;
			}
		}
		else
			break;
	}
	return;
}

/*	检查是否需要reset	*/
PUBLIC int check_wait_queue(int *queue)
{
	int i, j = 0, k;

	for (i = 0; i < NR_TASKS; i++)
	{
		if (queue[i] == -1)
			continue;
		if (proc_table[queue[i]].times > 0)
		{
			return 0;
		}
	}

	/*	代表只剩下F进程剩余次数，则主动reset	*/
	reset(queue);

	for (i = 0; i < NR_TASKS; i++)
	{
		if (proc_table[queue[i]].sleep_time == 0)
		{
			int ready_proc = dequeue(queue);
			refresh(queue);
			PROCESS *p_ready = &proc_table[ready_proc];
			p_ready->state = 0x0002;
			enqueue(p_ready, ready_queue);
		}
	}

	schedule();

	return 1;
}

/*	刷新屏幕	*/
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
```

> proc.c

```c
/*	进程调度函数	*/
PUBLIC void schedule()
{
	int j, k;

	/*	直接从就绪队列中拿出进程，能不能运行应该由PV操作去互斥或同步	*/
	if (ready_queue[0] >= 0)
	{
		if (p_proc_ready->state == 0x0003)
		{
			if (p_proc_ready->type == 0x0003 && isInReadyQ(p_proc_ready->pid) == 1)
			{
				j = dequeue(ready_queue);
				refresh(ready_queue);
				p_proc_ready = &proc_table[j];
				return;
			}
			enqueue(p_proc_ready, ready_queue);
			j = dequeue(ready_queue);
			refresh(ready_queue);
			p_proc_ready = &proc_table[j];
		}
		else if (p_proc_ready->state == 0x0001)
		{
			/*	in wait_queue表示被互斥	*/
			j = dequeue(ready_queue);
			refresh(ready_queue);
			p_proc_ready = &proc_table[j];
		}
		/*	出现在reset后全部进程都置为ready且还未有进程开始跑	*/
		else if (p_proc_ready->state == 0x0002)
		{
			j = dequeue(ready_queue);
			refresh(ready_queue);
			p_proc_ready = &proc_table[j];
		}
		else{

		}
	}
	else
	{
		/* code */
	}

	return;
}

/*	以下为PV操作的系统调用函数	*/
PUBLIC void sys_p_operation(SEMAPHORE *semaphore)
{
	semaphore->value--;
	while (semaphore->value < 0 && semaphore == &mutex){}
	if (semaphore == &read){
		if (rc >= LIMIT_READ){
			sleep(wait_queue);
		}
	}
	if (semaphore->value < 0 && semaphore == &write)
	{
		sleep(wait_queue);
	}
}

PUBLIC void sys_v_operation(SEMAPHORE *semaphore)
{
	semaphore->value++;

	if (semaphore == &write)
	{
		int i = check_wait_queue(wait_queue);
		if (semaphore->value <= 0)
		{
			if (i == 0)
			{
				wakeUp(wait_queue);
			}
		}
	}

	if (semaphore == &read){
		if (rc < LIMIT_READ){
			/*	释放wait_queue中的read进程	*/
			refresh(wait_queue);
			int i;
			for (i = 0; i < NR_TASKS; i++){
				if (proc_table[wait_queue[i]].times > 0 && proc_table[wait_queue[i]].type == 0x0001){
					int j = dequeue(&wait_queue[i]);
					refresh(wait_queue);

					proc_table[j].state == 0x0002;
					enqueue(&proc_table[j], ready_queue);

					schedule();
					break;
				}
			}
		}
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

	int i;
	for (i = 0; i < NR_TASKS; i++)
	{
		if (proc_table[queue[i]].times > 0 && proc_table[queue[i]].sleep_time == 0)
		{
			int ready_proc = dequeue(&queue[i]);
			refresh(queue);
			PROCESS *p_ready = &proc_table[ready_proc];
			p_ready->state = 0x0002;
			enqueue(p_ready, ready_queue);

			return;
		}
	}

	return;
}
```

> 结构体

```c
/*	信号量	*/
typedef	struct semaphore {
	int	value;
	int	queue[NR_TASKS];
}SEMAPHORE;

/*	进程	*/
typedef struct s_proc {
	STACK_FRAME regs;          /* process registers saved in stack frame */

	u16 ldt_sel;               /* gdt selector giving ldt base and limit */
	DESCRIPTOR ldts[LDT_SIZE]; /* local descriptors for code and data */

    int	proc_time;		/*	ticks per cycle	*/
	int	ticks;			/*	remained ticks	*/
	int sleep_time;
	int times;
	u32	state;			/*	0:default; 1: wait; 2: ready; 3: on processing	*/
	u32 type;			/*	0:default; 1: reader; 2: writer; 3: simple	*/


	u32 pid;                   /* process id passed in from MM */
	char p_name[16];           /* name of the process */
}PROCESS;
```

