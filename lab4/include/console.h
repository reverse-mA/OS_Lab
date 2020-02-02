
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_


/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
}CONSOLE;

PUBLIC CONSOLE* current_con;

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define WHITE_COLOR	0x07 //白色
#define CYAN_COLOR 0x03 //青色
#define DEEP_RED_COLOR 0x0D //深红
#define RED_COLOR 0x05   //红色
#define GREEN_COLOR 0x02 //绿色
#define BROWN_COLOR 0x0E //棕色
#define BLUE_COLOR	0x01 //蓝色


#endif /* _ORANGES_CONSOLE_H_ */
