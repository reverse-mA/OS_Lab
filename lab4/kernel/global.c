
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PUBLIC	PROCESS		proc_table[NR_TASKS];

PUBLIC	char		task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "tty"},
					                    {ReaderA, STACK_SIZE_ReaderA, "ReaderA"},
										{ReaderB, STACK_SIZE_ReaderB, "ReaderB"},
										{ReaderC, STACK_SIZE_ReaderC, "ReaderC"},
										{WriterD, STACK_SIZE_WriterD, "WriterD"},
										{WriterE, STACK_SIZE_WriterE, "WriterE"},
										{NormalF, STACK_SIZE_NormalF, "NorlmalF"}
									};

PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];

PUBLIC	irq_handler	irq_table[NR_IRQ];

PUBLIC	system_call	sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_sleep, sys_print,  sys_p, sys_v};
