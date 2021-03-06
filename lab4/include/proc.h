
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


typedef struct s_stackframe {	/* proc_ptr points here				↑ Low			*/
	u32	gs;		/* ┓						│			*/
	u32	fs;		/* ┃						│			*/
	u32	es;		/* ┃						│			*/
	u32	ds;		/* ┃						│			*/
	u32	edi;		/* ┃						│			*/
	u32	esi;		/* ┣ pushed by save()				│			*/
	u32	ebp;		/* ┃						│			*/
	u32	kernel_esp;	/* <- 'popad' will ignore it			│			*/
	u32	ebx;		/* ┃						↑栈从高地址往低地址增长*/		
	u32	edx;		/* ┃						│			*/
	u32	ecx;		/* ┃						│			*/
	u32	eax;		/* ┛						│			*/
	u32	retaddr;	/* return address for assembly code save()	│			*/
	u32	eip;		/*  ┓						│			*/
	u32	cs;		/*  ┃						│			*/
	u32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	u32	esp;		/*  ┃						│			*/
	u32	ss;		/*  ┛						┷High			*/
}STACK_FRAME;


typedef struct s_proc {
	STACK_FRAME regs;          /* process registers saved in stack frame */

	u16 ldt_sel;               /* gdt selector giving ldt base and limit */
	DESCRIPTOR ldts[LDT_SIZE]; /* local descriptors for code and data */

        int ticks;                 /* remained ticks */
        int priority;

	u32 pid;                   /* process id passed in from MM */
	char *p_name;           /* name of the process */
 
    int sleep; //睡眠时间片
	int stop; //0表示没被阻塞；1表示被阻塞
	struct s_proc* next; //下一个进程
	int segment;//占用的时间片
	int type;//进程的类型，0表示读者，1表示写者
	int last_type;//上一个进程的类型，用于调度到F进程时使用
	int color;//进程对应的颜色
}PROCESS;

typedef struct s_task {
	task_f	initial_eip;
	int	stacksize;
	char	name[32];
}TASK;

//semaphore
typedef struct semaphore {
	int value;
	PROCESS* queue;
}SEMAPHORE;

/* Number of tasks */
#define NR_TASKS	7

/* stacks of tasks */
#define STACK_SIZE_TTY		0x8000
#define STACK_SIZE_ReaderA	0x8000
#define STACK_SIZE_ReaderB	0x8000
#define STACK_SIZE_ReaderC   0x8000
#define STACK_SIZE_WriterD   0x8000
#define STACK_SIZE_WriterE   0x8000
#define STACK_SIZE_NormalF   0x8000

#define STACK_SIZE_TOTAL	(STACK_SIZE_TTY + \
				STACK_SIZE_ReaderA + \
				STACK_SIZE_ReaderB + \
				STACK_SIZE_ReaderC + \
				STACK_SIZE_WriterD + \
				STACK_SIZE_WriterE + \
				STACK_SIZE_NormalF)

