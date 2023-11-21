
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_get_delay		equ 1
_NR_print_str		equ 2

_NR_p_operation		equ	3
_NR_v_operation		equ	4
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global  get_delay
global	print_str
global	p
global	v

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              get_delay
; ====================================================================
get_delay:
	mov eax, _NR_get_delay
	mov esi, esp
	mov ebx, [esi + 4]
	int	INT_VECTOR_SYS_CALL
	ret
; ====================================================================
;                              print_str
; ====================================================================
print_str:
	mov eax, _NR_print_str
	mov	esi, esp
	mov ebx, [esi + 4]		; char* str : 32bit
	int INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              PV操作
; ====================================================================
p:
	mov	eax,	_NR_p_operation
	mov	esi, esp
	mov	ebx, [esi + 4]
	int	INT_VECTOR_SYS_CALL
	ret

v:
	mov	eax,	_NR_v_operation
	mov	esi, esp
	mov	ebx, [esi + 4]
	int	INT_VECTOR_SYS_CALL
	ret
