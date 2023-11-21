
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
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
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;

	int i,j;
	for (i = 0; i < NR_TASKS ; i++){
		if (proc_table[i].sleep_time != 0 && proc_table[i].state == 0x0001){
			proc_table[i].sleep_time--;
			if (proc_table[i].sleep_time == 0){
				proc_table[i].state = 0x0002;
				if (proc_table[i].times > 0){
					refresh(ready);
					enqueue(&proc_table[i], ready);
				}
			}
		}
	}

	if (k_reenter != 0) {
		return;
	}

	schedule();

}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{       
	int t = get_ticks();

    while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*======================================================================*
                              doSomething
							  不调用时间片
 *======================================================================*/
PUBLIC void doSomething(int milli_sec)
{
	milli_delay(milli_sec);

	get_delay(milli_sec);

	execution++;
}