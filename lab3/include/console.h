
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */

#define BLUE_COLOR	0x71;	//黑底蓝字
//每一个命令，不包括ctrl+z
typedef struct s_command{
    //命令对应的字符
    char buffer;
    //字符被输出之前的光标位置
    int buffer_cursor;
    //如果该字符是退格符，还会额外记录删去的字符
    char delete_buffer;
}Command;

/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
	unsigned int    state;/*当前控制台所处的状态，0表示正常，1表示搜索，2表示只接受esc*/
	unsigned int    index;/*当前命令list的下标*/
	Command command_list[SCREEN_SIZE];//维护状态0下输出的所有命令(ESC除外)的数组
	unsigned int    state0_cursor;//记录state0下的光标，方便从状态1和状态2中返回
	char target[100];//记录被查找的字符串
	unsigned int len;//记录被查找的字符串的长度

}CONSOLE;



#endif /* _ORANGES_CONSOLE_H_ */
