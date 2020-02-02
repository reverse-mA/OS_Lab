
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"
#include <stdlib.h>

PRIVATE char buffer[10];
PRIVATE int now;
PRIVATE SEMAPHORE 	semaphores[6] = {{1, 0},{1, 0},{1, 0},{1, 0},{1, 0},{3,0}
              									//rmutex, wmutex,x,y,z
												};
int readcount,writecount;
int hasPrint;//用于让第一次不要调度到F进程，初值为0
int time;
SEMAPHORE* wmutex;
SEMAPHORE* rmutex;
SEMAPHORE* x;
SEMAPHORE* y;
SEMAPHORE* z;
SEMAPHORE* readerLimit;
/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	for (p = proc_table; p < proc_table+NR_TASKS; p++) {
		if(p -> sleep > 0){
			p -> sleep--;
		}
	}

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p -> stop || p -> sleep){
				continue;
			}

			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
				break;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if (p -> stop || p -> sleep){
					continue;
				}
				p->ticks = p->priority;
			}
		}
	}

	//如果打印进程没有在睡，就应该调度他
	if(proc_table[6].sleep==0){
		//保存上一个进程的类型
		if(hasPrint){
			proc_table[6].last_type = p_proc_ready->type;
			p_proc_ready = &proc_table[6];
		}
		else{
			hasPrint = 1;//从第二次开始调度
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                           sys_process_sleep
 *======================================================================*/
PUBLIC int sys_sleep(int milli_sec){
	p_proc_ready -> sleep = milli_sec *HZ/1000;
	schedule();
	return 0;
}

/*======================================================================*
                           sys_disp_str
 *======================================================================*/
PUBLIC int sys_print(char* s){

    while (*s != 0){
        out_char(current_con, *s, p_proc_ready->color);
        s++;
    }

    return 0;
}

/*======================================================================*
                           sys_P
 *======================================================================*/
PUBLIC int sys_p(SEMAPHORE* s){
	s->value--;
	if(s->value < 0){
		//阻塞进程
		p_proc_ready->stop = 1;
		if(s -> queue == 0){
			s->queue = p_proc_ready;
		}
		else{
			PROCESS* p = s->queue;
			while(p->next != 0){
				p = p -> next;
			}
			p->next = p_proc_ready;
		}
		schedule();
	}
	return 0;
}

/*======================================================================*
                           sys_V
 *======================================================================*/
PUBLIC int sys_v(SEMAPHORE* s){
	s->value++;
	if(s->value <= 0){
		//唤醒进程
		p_proc_ready = s->queue;
		s->queue = s-> queue -> next;
		p_proc_ready->next = 0;
		p_proc_ready->stop = 0;
		
	}
	return 0;
}

/*======================================================================*
                           init
 *======================================================================*/
PUBLIC void init(){
	readcount = 0;
	writecount = 0;
	hasPrint = 0;
	time = 10000;
	rmutex=&semaphores[0];
    wmutex=&semaphores[1];
    x=&semaphores[2];
	y=&semaphores[3];
	z=&semaphores[4];
	readerLimit=&semaphores[5];
}

/*======================================================================*
                              读者优先情况下，读者算法
*======================================================================*/
PUBLIC void readerF_reader(){
    print(p_proc_ready->p_name);
    print(" starts.\n");
	P(readerLimit);
    P(rmutex);
    readcount++;
    if(readcount==1){
		P(wmutex);
	}
    V(rmutex);
    print(p_proc_ready->p_name);
    print(" is running now.\n");
    milli_delay(p_proc_ready->segment*time);
	print(p_proc_ready->p_name);
    print(" ends.\n");
	P(rmutex);
	readcount--;
	if(readcount==0){
    	V(wmutex);
    }	
    V(rmutex);
	V(readerLimit);
}
/*======================================================================*
                              读者优先情况下，写者算法
*======================================================================*/
PUBLIC void readerF_writer(){
    print(p_proc_ready->p_name);
    print(" starts.\n");
    P(wmutex);
    print(p_proc_ready->p_name);
    print(" is running now.\n");
	int i=1;
    milli_delay(p_proc_ready->segment*time);
	print(p_proc_ready->p_name);
    print(" ends.\n");
    V(wmutex);
}

/*======================================================================*
                              写者优先情况下，读者算法
*======================================================================*/
PUBLIC void writerF_reader(){
	P(readerLimit);
    P(z);
	print(p_proc_ready->p_name);
    print(" starts.\n");
	P(rmutex);
	P(x);
    readcount++;
    if(readcount==1){
		P(wmutex);
	}
	V(x);
	V(rmutex);
	V(z);
    print(p_proc_ready->p_name);
    print(" is running now.\n");
    milli_delay(p_proc_ready->segment*time);
	print(p_proc_ready->p_name);
    print(" ends.\n");
    P(x);
    readcount--;
    if(readcount==0){
    	V(wmutex);
    }
    V(x);
	V(readerLimit);
}
/*======================================================================*
                              写者优先情况下，写者算法
*======================================================================*/
PUBLIC void writerF_writer(){
    print(p_proc_ready->p_name);
    print(" starts.\n");
    P(y);
	writecount++;
	if(writecount==1)P(rmutex);
	V(y);
	P(wmutex);
    print(p_proc_ready->p_name);
    print(" is running now.\n");
    milli_delay(p_proc_ready->segment*time);
	print(p_proc_ready->p_name);
    print(" ends.\n");
    V(wmutex);
	P(y);
	writecount--;
	if(writecount==0)V(rmutex);
	V(y);
}

//输出当前进程是读者占用还是写者占用，是读者占用的话有多少个
PUBLIC void output(){
	//说明此时由写者占用
	if(p_proc_ready->last_type==1){
		print("Writer Occupy!\n");
	}
	else if(p_proc_ready->last_type==0){
		//说明此时由读者占用
		print("Reader Occupy! Num is ");
		char temp = (char)(readcount+48);
    	print(&temp);
		print("\n");
	}
	sleep(p_proc_ready->segment*time);
}

