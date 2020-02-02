
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	//每次都只分给一个时间片
	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = 1;
	proc_table[4].ticks = proc_table[4].priority = 1;
	proc_table[5].ticks = proc_table[5].priority = 1;
	proc_table[6].ticks = proc_table[6].priority = 1;

	proc_table[0].stop = proc_table[0].sleep = 0;
	proc_table[1].stop = proc_table[1].sleep = 0;
	proc_table[2].stop = proc_table[2].sleep = 0;
	proc_table[3].stop = proc_table[3].sleep = 0;
	proc_table[4].stop = proc_table[4].sleep = 0;
	proc_table[5].stop = proc_table[5].sleep = 0;
	proc_table[6].stop = proc_table[6].sleep = 0;

	proc_table[0].next = 0;
    proc_table[1].next = 0;
	proc_table[2].next = 0;
	proc_table[3].next = 0;
	proc_table[4].next = 0;
	proc_table[5].next = 0;
	proc_table[6].next = 0;

	proc_table[0].p_name = "start";
    proc_table[1].p_name = "ReaderA";
    proc_table[2].p_name = "ReaderB";
    proc_table[3].p_name = "ReaderC";
    proc_table[4].p_name = "WriterD";
    proc_table[5].p_name = "WriterE";
    proc_table[6].p_name = "NormalF";

	//进程跑完需要的时间片
	proc_table[0].segment = 0;
	proc_table[1].segment = 2;
    proc_table[2].segment = 3;
    proc_table[3].segment = 3;
    proc_table[4].segment = 3;
    proc_table[5].segment = 4;
    proc_table[6].segment = 1;

	//进程的类型
	proc_table[0].type = -1;
	proc_table[1].type = 0;
    proc_table[2].type = 0;
    proc_table[3].type = 0;
    proc_table[4].type = 1;
    proc_table[5].type = 1;
    proc_table[6].type = 2;

	//初始化为-1
	proc_table[0].last_type = -1;
	proc_table[1].last_type = -1;
    proc_table[2].last_type = -1;
    proc_table[3].last_type = -1;
    proc_table[4].last_type = -1;
    proc_table[5].last_type = -1;
    proc_table[6].last_type = -1;

	//初始化每个进程的颜色
	proc_table[0].color = RED_COLOR;
	proc_table[1].color = DEEP_RED_COLOR;
	proc_table[2].color = CYAN_COLOR;
	proc_table[3].color = BROWN_COLOR;
	proc_table[4].color = GREEN_COLOR;
	proc_table[5].color = BLUE_COLOR;
	proc_table[6].color = WHITE_COLOR;
	

	init();
	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
    init_keyboard();
	restart();

	while(1){}
}

/*======================================================================*
                               ReaderA
 *======================================================================*/
void ReaderA()
{
	while(1){
	// readerF_reader();
	// sleep(10000);
	writerF_reader();
	sleep(10000);
	}
}

/*======================================================================*
                               ReaderB
 *======================================================================*/
void ReaderB()
{
	while(1){
	//readerF_reader();
	// sleep(10000);
	writerF_reader();
	sleep(10000);
	}
}

/*======================================================================*
                               ReaderC
 *======================================================================*/
void ReaderC()
{
	while(1){
	//readerF_reader();
	// sleep(10000);
	writerF_reader();
	sleep(10000);
	}
}

/*======================================================================*
                               WriterD
 *======================================================================*/
void WriterD()
{
	while(1){
	//readerF_writer();
	// sleep(10000);
	writerF_writer();
	sleep(50000);
	}
}

/*======================================================================*
                               WriterE
 *======================================================================*/
void WriterE()
{
	while(1){
	//readerF_writer();
	//sleep(10000);
	writerF_writer();
	sleep(50000);
	}
}

/*======================================================================*
                               NormalF
 *======================================================================*/
void NormalF()
{
	while(1){
	output();
	}
}
