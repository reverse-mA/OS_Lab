
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void ReaderA();
void ReaderB();
void ReaderC();
void WriterD();
void WriterE();
void NormalF();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch, char color);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);

/* 以下是系统调用相关 */

/* proc.c */
PUBLIC  int     sys_get_ticks();
PUBLIC int sys_sleep(int milli_sec);  
PUBLIC  int 	sys_print(char* str);
PUBLIC int sys_p(SEMAPHORE* s);
PUBLIC int sys_v(SEMAPHORE* s);
PUBLIC void init();
PUBLIC void readerF_reader();
PUBLIC void readerF_writer();
PUBLIC void writerF_reader();
PUBLIC void writerF_writer();
PUBLIC void output();


/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC int P(SEMAPHORE*s);
PUBLIC int V(SEMAPHORE*s);
PUBLIC int print(char* str);
PUBLIC int sleep(int milli_sec);