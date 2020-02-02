
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 
_NR_sleep	equ 1 ;
_NR_print      equ 2 ; 
_NR_P 		equ 3 ;
_NR_V		equ 4 ;
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global	sleep
global  print
global	P
global  V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	push ebx
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret

; ====================================================================
;                              sleep
; ====================================================================
sleep:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_sleep
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret

; ====================================================================
;                              output
; ====================================================================
print:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_print
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret

	; ====================================================================
;                              P
; ====================================================================
P:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_P
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret

; ====================================================================
;                              V
; ====================================================================
V:
	push ebx
	mov ebx, [esp + 8]
	mov	eax, _NR_V
	int	INT_VECTOR_SYS_CALL
	pop ebx
	ret