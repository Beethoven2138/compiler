section .data
section .bss
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 16
_scope_start1:
	MOV R14, [RBP-16]	;R14 = ptr;
	MOV R15, [R14]		;R15 = *ptr;
	MOV R14, 2
	ADD R15, R14		;R15 = *ptr + 2;
	MOV [RBP-24], R15	;var = *ptr + 2;
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
