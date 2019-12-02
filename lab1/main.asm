global _start;

section .data
msg1 db 'please input x and y: ',0h
msg2 db ''

section .bss
inputStr: resb 50 ; 允许读入50个字节
result: resb 50

section .text
_start:
mov eax, msg1 ; 打印输出提示
call sprintLF

mov ecx, inputStr ; 事先准备好的用于存字符的内存地址
mov edx, 50 ; 告诉函数最多读50个
call input ; 调用输入函数

mov eax, inputStr ; 现在eax里是输入的字符串的首字母地址
call nextSpace ; 调用完后esi的值是下一个数字的开始地址
mov edi, eax ; edi存第一个字符串的首字母地址
;到这个时候,edi的值是第一个字符串的首字母地址，esi的值是第二个字符串的首字母地址
call tail
;edi存的值是第一个字符串的首地址，esi存的值是第二个字符串的首地址，执行后ecx存第一个字符串的尾地
;址，ebx存第二个字符串的尾地址

call add;
mov eax,msg2 
call sprintLF ; 打印分隔符加换行
call char2int
call mul;
call quit ; 

;把首地址到尾地址中所有的数字转换为int,ebx是第一个数字的尾地址,edx第二个尾，edi第一个首,esi第二个首
char2int:
push ebx
push edx
push edi
push esi
dec edi;
dec esi;
inc ebx
inc edx

f2Int:
dec ebx
cmp ebx,edi
je s2Int ;
cmp byte[ebx],30h
jb f2Int
sub byte[ebx],30h
jmp f2Int

s2Int:
dec edx
cmp edx,esi
je finish2Int
cmp byte[edx],30h
jb s2Int
sub byte[edx],30h
jmp s2Int

finish2Int:
pop esi
pop edi
pop edx
pop ebx
ret

;一个字符一个字符的输出
printOne:
push eax
push ebx
push ecx
push edx
mov     ecx, eax ; ecx存开始地址
mov     eax, 4 ; 表示调用sys_write
mov     ebx, 1 ; 表示写到控制台
mov     edx, 1 ; 表示只打一位
int     80h ; 调用函数
pop edx
pop ecx
pop ebx
pop eax
ret

; 直接往edx里装字符串长度
slen:
push    eax
mov     edx, eax


nextchar:
cmp     byte [edx], 0 ; 如果读到0, 置zf
jz      finished ; 如果zf被置，调用finished
inc     edx ; 相当于取下一个地址，字节
jmp     nextchar
 
finished:
sub     edx, eax ; 此时edx存字符串长度
pop     eax
ret
 
; 打印字符串，调用时eax存字符串开始地址
sprint:
; 进栈
push    edx
push    ecx
push    ebx
push    eax
; 调用计算字符串长度的函数，成功后edx的值为字符串长度
call    slen
; 调用系统函数
mov     ecx, eax ; ecx存开始地址
mov     eax, 4 ; 表示调用sys_write
mov     ebx, 1 ; 表示写到控制台
int     80h ; 调用函数

; 出栈
pop     eax 
pop     ebx
pop     ecx
pop     edx
ret

;打印字符串并换行，调用时eax存字符串开始地址 
sprintLF:
call sprint ; 先打印字符串
push eax ; 进栈
mov eax, 0Ah ; 把换行符存在eax里
push eax ; 再进栈
mov eax, esp ; 把此时的栈顶指针给eax
call sprint ; 打印换行符
pop eax ; 不要换行符
pop eax ; 恢复原值
ret

; 退出函数 
quit:
mov     eax, 1 ; 表示调用sys_exit
mov     ebx, 0 ; 表示no_error
int     80h
ret

; 调用输入函数，需传ecx:开始地址和edx:长度
input:
push eax
push ebx
mov eax, 3 ; 表示调用sys_read
mov ebx, 0 ; 读键盘缓冲区里的东西
int 80h ; 中断调用

pop ebx
pop eax
ret

;查找下一个空格地址,顺便把空格改成0h，需传eax开始地址,结束后在esi中存储空格地址的下一个地址
nextSpace:
push edx
mov edx, eax ; edx存开始地址

nextAddress:
cmp byte[edx],32 ; 检查edx里的值是不是空格，是的话置zf
jz finishSpace ; 是的话跳转
inc edx ; edx取下个地址
jmp nextAddress ;循环

finishSpace:
mov byte[edx],0h ; 把地址里的值改成0h，方便检查
inc edx
mov esi, edx ; 此时esi里的值就是下一个数字的开始地址
pop edx
ret

;edi存的值是第一个字符串的首地址，esi存的值是第二个字符串的首地址，执行后ebx存第一个字符串的尾地址，edx存第二个字符串的尾地址
tail:
mov ebx, esi
sub ebx,2; 第一个字符串的尾地址是第二个字符串的首地址上面2个,ebx中存了第一个字符串的尾地址
push eax
;mov eax, ebx
;call sprint
mov eax,esi
call slen ; 此时edx存第二个字符串的长度（包括换行）
sub edx,2 ; 不要换行,加字符串长度-1
add edx, esi ; 此时edx存第二个字符串的尾字母地址
;mov eax, edx
;call sprint
pop eax
ret

; 根据tail完成后的寄存器状态，进行加法运算,执行后ebx存第一个字符串的尾>地址，edx存第二个字符串的尾地址,ecx用来保存进位
add:
; 寄存器状态保存
push ebx ; 之后会用到ebx，先push
push edx ; 之后会用到edx，先push
push edi
push esi
dec edi ; 第一个字符串的首地址前一个
dec esi ; 第二个字符串的首地址前一个
mov eax,0h
push eax ;用于标识结束符
mov ecx,0d ; 初始化ecx
addLoop:
sub byte[ebx],30h ; 将ebx地址中的值转为int
sub byte[edx],30h ; 将edx地址中的值转为int
mov eax,[ebx] ; 把ebx地址里的值给eax
add eax,[edx] ; eax = eax+[edx]
add eax,ecx ; eax加进位
mov ecx,0d ; 再初始化
cmp al,10
jb  nextAlpha ; 没有进位跳转到nextAlpha
add ecx,1d ; 进位寄存器置为1
sub eax,10d

nextAlpha: 
add eax,30h ; i2c
push eax ; 直接push进栈
dec ebx
dec edx
cmp ebx,edi ; 看第一个字符串是否读到头
jz finishF ; 是的话跳转到finishF
cmp edx,esi ; 看第二个字符串是否处理完
jz finishS ; 是的话跳转到finishS
jmp addLoop ; 都没有就继续加

finishF:
cmp edx,esi ; 看第二个字符串是否处理完
je finishFS ; 是的话跳转到finishFS
mov eax,[edx]
add eax,ecx
mov ecx,0d
cmp al,58d
jb  nextAlphaF ; 没有进位跳转到nextAlpha
add ecx,1d ; 进位寄存器置为1
sub eax,10d

nextAlphaF:
push eax
dec edx
jmp finishF

finishS:
cmp ebx,edi ;看第一个字符串是否处理完
je finishFS ; 是的话跳转到finishFS
mov eax,[ebx]
add eax,ecx
mov ecx,0d
cmp al,58d
jb  nextAlphaS
add ecx,1d ; 进位寄存器置为1
sub eax,10d

nextAlphaS:
push eax
dec ebx
jmp finishS

finishFS:
cmp ecx,0
je finishFSJ
add ecx,30h
push ecx

finishFSJ:
lea eax,[esp]
mov     ecx, eax ; ecx存开始地址
mov     eax, 4 ; 表示调用sys_write
mov     ebx, 1 ; 表示写到控制台
mov     edx, 1 ;一次只打印一位
int     80h ; 调用函数
mov eax,[esp] ; 看是否到结束符
cmp eax,0h
jz finishAll
pop eax ; pop栈顶
jmp finishFSJ

finishAll:
pop eax ; 把结束符pop
pop esi
pop edi
pop edx ; 把edx的值还回来
pop ebx ; 把ebx的值还回来
ret

;eax中存被除数的值，除以10，结束后bl中存商，dl中存余数
div10:
push eax
push ecx
mov bl,0
mov dl,0
divLoop:
cmp al,10d
jb finishDiv
add bl,1
sub al,10
jmp divLoop

finishDiv:
mov dl,al
pop ecx
pop eax
ret

mul:
push ebx
push edx
push edi
push esi
dec edi ; 让edi存第一个数字的首地址前一个
dec esi ; 让esi 第二个数字的首地址前一个
push ebx ; 存ebx
push edx ; 存edx
sub ebx,edi ;
inc ebx ; 此时ebx存第一个数字的长度
sub edx,esi
inc edx ; 此时edx存第二个数字的长度
add ebx,edx ; ebx存两个数字的长度
mov ecx,ebx ; ecx存两个数字的长度
pop edx ;恢复edx
pop ebx;恢复ebx 
;将尾位址到开始地址之间的值清0
push ecx ; 把数字长度和进栈
dec ecx ; 纯粹为了好清0
all0:
cmp ecx,-1d
je coutinue ; 此时所有值清0，跳转
mov eax,0d
mov [result+ecx],eax
dec ecx
jmp all0
coutinue:
mov ecx, 0d ; ecx用于count
;出栈
outLoop:
push ecx ; 外层循环数进栈
push edx ; 二号数字尾地址进栈
innerLoop:
mov eax,[ebx] ; 把ebx地址里的值给eax
;ebx里的值是对的
mul byte[edx] ; eax*[edx],eax里的值是低16位，但只要低16位，因为是个位数乘法
;乘起来的值也是对的,ecx指向对
add eax,[result+ecx]
push edx
push ebx
call div10
mov [result+ecx],dl ; 在地址里存余数
inc ecx ; ecx+=1
add [result+ecx],bl; 在下一个地址里存进位
pop ebx
pop edx
dec edx ;看第二个字符串到头没 
cmp edx,esi
jne innerLoop ; 没到头就继续在内层循环
pop edx ; 二号数字尾地址出栈，之后还会使用
pop ecx ; 外层循环数出栈
inc ecx ; 外层循环加1
dec ebx ; 看第一个字符串到头没
cmp ebx,edi
je finishMul ; 到头了就跳出
jmp outLoop
finishMul:
pop ecx ; 长度出栈
dec ecx
dec ecx
dec ecx
mov eax,result ;把新空间地址给eax
add eax,ecx

;把首位的0处理掉
remove0:
cmp byte[eax],0d ; 看看首位是不是0
jne printLoop ; 不是直接跳转
cmp eax,result ; 看看是不是到最后一位
je printLoop ; 是的话直接打印0
dec eax
jmp remove0

printLoop:
add byte[eax],30h
call printOne
dec eax
cmp eax,result
jnb printLoop

finishMulP:
pop esi
pop edi
pop edx
pop ebx
ret

