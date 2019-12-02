SECTION .DATA
color_default:  db  1Bh, '[37;0m', 0
.len            equ $ - color_default
color_red:      db  1Bh, '[31;1m', 0
.len            equ $ - color_red
message         db  'Hello World!!!', 0Ah
linefeed        db  0Ah
.len            equ $ - linefeed
colon           db  58d
.len            equ $ - colon
space           db  32d
.len            equ $ - space
point           db  46d
.len            equ $ -point

SECTION .text
global printBlack
global printRed
global print
global printLineFeed
global printSpace
global printColon
global printPoint
global iprint
global printACluster

printBlack:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, color_default
    mov     edx, color_default.len
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

printRed:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, color_red
    mov     edx, color_red.len
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

slen:
    push    ebx
    mov     ebx, eax
 
nextchar:
    cmp     byte [eax], 0
    jz      finished
    inc     eax
    jmp     nextchar
 
finished:
    sub     eax, ebx
    pop     ebx
    ret
 
 
printACluster:
push    eax
push    ebx
push    ecx
push    edx

mov     ecx, [esp+20]
mov     ebx, 1
mov     eax, 4
mov     edx, 512
int     80h
pop     edx
pop     ecx
pop     ebx
pop     eax
ret

;------------------------------------------
; void sprint(String message)
; String printing function
print:
    call    slen
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     edx, eax
 
    mov     ecx, [esp+20]
    mov     ebx, 1
    mov     eax, 4
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

printLineFeed:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, linefeed
    mov     edx, linefeed.len
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret 

printSpace:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, space
    mov     edx, space.len
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret 

printColon:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, colon
    mov     edx, colon.len
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

printPoint:
    push    eax
    push    ebx
    push    ecx
    push    edx
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, point
    mov     edx, point.len
    int     80h
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

sprint:
    push    edx
    push    ecx
    push    ebx
    push    eax
    call    slen
 
    mov     edx, eax
    pop     eax
 
    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h
 
    pop     ebx
    pop     ecx
    pop     edx
    ret

iprint:
    push    eax             
    push    ecx             
    push    edx             
    push    esi             
    mov     ecx, 0
    mov     eax, [esp+20]          
 
divideLoop:
    inc     ecx             
    mov     edx, 0          
    mov     esi, 10         
    idiv    esi             
    add     edx, 48         
    push    edx             
    cmp     eax, 0          
    jnz     divideLoop      
 
printLoop:
    dec     ecx             
    mov     eax, esp        
    call    sprint          
    pop     eax             
    cmp     ecx, 0          
    jnz     printLoop       
 
    pop     esi             ; restore esi from the value we pushed onto the stack at the start
    pop     edx             ; restore edx from the value we pushed onto the stack at the start
    pop     ecx             ; restore ecx from the value we pushed onto the stack at the start
    pop     eax             ; restore eax from the value we pushed onto the stack at the start
    ret
