
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"
#include <string.h>

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE int getLastCursor(CONSOLE* p_con);
PRIVATE void backOut(CONSOLE* p_con);
PRIVATE char getLastChar(CONSOLE* p_con);
PRIVATE void back2state0(CONSOLE* p_con);
PRIVATE void seek(CONSOLE* p_con);
PRIVATE void restore(CONSOLE* p_con);

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
    //第几个tty
	int nr_tty = p_tty - tty_table;
	//tty对应的console指针
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	//初始console状态设为0
    p_tty->p_console->state = 0;
    //当前命令list的下标设为0
    p_tty->p_console->index = 0;
    //初始被查找的数组长度为0
    p_tty->p_console->len = 0;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

    memset(p_tty->p_console->target,'\0', sizeof(char)*100);
	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
    //这个是当前指令的下标
    int index= p_con->index;
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	//存储状态0下的非撤销命令和非ESC命令
	if(ch!=5&&ch!=27&&p_con->state==0) {
        Command *command = &(p_con->command_list[p_con->index]);
        command->buffer = ch;
        command->buffer_cursor = p_con->cursor;
        p_con->index++;
    }
	switch(ch) {
	case '\n':
	    //状态0下换行
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH&&p_con->state==0) {
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
		}
		//状态1下按下回车，进入状态2
		else if(p_con->state==1){
		    seek(p_con);
		    p_con->state=2;
		}
		break;
	case '\b':
	    //只有状态0可以退格
		if (p_con->cursor > p_con->original_addr&&p_con->index>0&&p_con->state==0) {
		    //获取上一个光标
		    int last_cursor = getLastCursor(p_con);
		    //这个是当前指令，需要把退格的字符存进去
            Command *command = &(p_con->command_list[index]);
            //获取上一个被删掉的字符
            command->delete_buffer = getLastChar(p_con);
		    while(p_con->cursor>last_cursor){
                p_con->cursor--;
                *(p_vmem-2) = '\0';
                *(p_vmem-1) = DEFAULT_CHAR_COLOR;
                p_vmem-=2;
		    }
		}
		break;

		//对TAB键进行处理
		case '\t':
            if (p_con->cursor <
                p_con->original_addr + p_con->v_mem_limit - 1) {
                //0状态下打印字符即可
                if(p_con->state==0||p_con->state==1) {
                    int i = 3;
                    while (i > -1) {
                        *p_vmem++ = '\0';
                        *p_vmem++ = DEFAULT_CHAR_COLOR;
                        p_con->cursor++;
                        i--;
                        //1状态下存储该字符
                        if(p_con->state==1){
                            //存储该字符
                            p_con->target[p_con->len] = '\0';
                            p_con->len++;
                        }
                    }
                }
            }
            break;

            //对ESC键做处理，这个地方只处理状态0和状态1下的ESC键
	    case 27:
	        //如果当前状态是1，给他改为0，并把屏幕上搜索输出的内容清空；如果当前状态是0，给他改为1，并记录此时的光标；
            if(p_con->state ==0) {
                p_con->state = 1;
                p_con->state0_cursor = p_con->cursor;
            }
            else if(p_con->state ==1) {
                back2state0(p_con);
            }
            break;


        case 5:
            //只有状态0可以撤销
            if(p_con->state ==0)backOut(p_con);
            break;


	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			if(p_con->state==0)*p_vmem++ = DEFAULT_CHAR_COLOR;
			else if(p_con->state==1){
			    *p_vmem++ = BLUE_COLOR;
			    //存储该字符
			    p_con->target[p_con->len] = ch;
			    p_con->len++;
			}
			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

PRIVATE void seek(CONSOLE* p_con){
    //光标指向开始
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->current_start_addr);
    int cursor = 0;
    int len = p_con->len;
    char *target = p_con->target;
    //如果根本没输入字符，直接返回就行
    if(len<1)return;
    while(cursor<=p_con->state0_cursor){
        //首先找到第一个字符就匹配的位置
        int i=0;
        int flag=1;
        while(i<len){
            if(cursor+i<=p_con->state0_cursor&&target[i]!=*(p_vmem+i*2)){
                flag=0;
                break;
            }
            i++;
        }
        i=0;
        if(!flag){
            cursor+=1;
            p_vmem+=2;
        }
        //说明有字符串匹配上了
        else{
            while(i<len){
                *(p_vmem+1)=BLUE_COLOR;
                cursor+=1;
                p_vmem+=2;
                i++;
            }
        }
        i=0;
    }
}

//查找模式复原
PRIVATE void restore(CONSOLE* p_con){
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->current_start_addr+1);
    int cursor = 0;
    while(cursor<p_con->state0_cursor){
        //迭代位
        *p_vmem = DEFAULT_CHAR_COLOR;
        p_vmem+=2;
        cursor++;
    }
}

PUBLIC void seekMode(CONSOLE* p_con, char ch){
    //这时候如果收到ESC就跳回到状态0，并回到初始状态
    if(ch==27){
        back2state0(p_con);
        restore(p_con);
    }
    while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
        scroll_screen(p_con, SCR_DN);
    }
    flush(p_con);
}

//返回状态0时的状态，将待查找的字符串数组清0，长度清0
PRIVATE void back2state0(CONSOLE* p_con){
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    p_con->state = 0;

    memset(p_con->target,'\0', sizeof(char)*100);
    //数组长度清0
    p_con->len = 0;
    while(p_con->cursor>p_con->state0_cursor){
        p_con->cursor--;
        *(p_vmem-2) = '\0';
        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
        p_vmem-=2;
    }
}

//撤销命令
PRIVATE void backOut(CONSOLE* p_con){

    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    //这个是当前指令的下标
    int index = p_con->index;
    //获取上一条的命令
    if(index>0) {
        Command *command = &(p_con->command_list[index - 1]);
        //获取打印这个字符前的光标位置
        int last_cursor = command->buffer_cursor;
        //如果这条命令不是退格命令
        if (command->buffer != '\b') {
            while (p_con->cursor > last_cursor) {
                p_con->cursor--;
                *(p_vmem - 2) = '\0';
                *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                p_vmem -= 2;
            }
        }
            //如果这条命令是退格命令
        else {
            //找到退格删掉的字符
            char ch = command->delete_buffer;
            if (p_con->cursor <
                p_con->original_addr + p_con->v_mem_limit - 1) {
                //如果这个字符不是tab键
                if (ch != '\t') {
                    *p_vmem++ = ch;
                    *p_vmem++ = DEFAULT_CHAR_COLOR;
                    p_con->cursor++;
                }
                    //如果是tab键
                else {
                    int i = 3;
                    while (i > -1) {
                        *p_vmem++ = '\0';
                        *p_vmem++ = DEFAULT_CHAR_COLOR;
                        p_con->cursor++;
                        i--;
                    }
                }
            }
        }
        //把这条命令释放掉
        memset(command, '\0', sizeof(Command));
        //避免野指针
        command = 0;
        p_con->index--;
    }
}
//获取上一个数字或字母的字符，没有则返回'\0'
PRIVATE char getLastChar(CONSOLE* p_con){
    int index = p_con->index;
    int i = index-1;
    int num = 0;
    while(i>-1){
        Command command = p_con->command_list[i];
        if(command.buffer!='\b'){
            num--;
            if(num<=0)return command.buffer;
        }
        else{
            num++;
        }
        i--;
    }
    return '\0';
}

//获取上一个数字或字母的光标位置，没有则输出-1
PRIVATE int getLastCursor(CONSOLE* p_con){
    int index = p_con->index;
    int i = index-1;
    int num = 0;
    while(i>-1){
        Command command = p_con->command_list[i];
        if(command.buffer!='\b'){
            num--;
            if(num<=0)return command.buffer_cursor;
        }
        else{
            num++;
        }
        i--;
    }
    return -1;
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

//清除当前屏幕，把光标移到左上方
PUBLIC void clear_console(CONSOLE* p_con){
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    //清除当前屏幕
    while(p_con->cursor>p_con->original_addr){
        p_con->cursor--;
        *(p_vmem-2) = '\0';
        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
        p_vmem -= 2;
    }
    //把光标移到开始位置
    set_cursor(p_con->original_addr);
}

//重设当前console中的缓存，这也就意味着不能再control+z之类的，也不能退格什么的，会调用clear_console
PUBLIC void reset_console(CONSOLE* p_con){
    //清除屏幕、移动光标
    clear_console(p_con);
    //把console的各种列表什么的初始化
    p_con->current_start_addr = p_con->original_addr;
    p_con->state = 0;
    p_con->index = 0;
    //把之前的列表项清掉
    memset(p_con->command_list,'\0', sizeof(Command)*SCREEN_SIZE);
}



