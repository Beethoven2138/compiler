section .data
section .bss
	arr: resq 10
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 0
_scope_start1:
	MOV R14, arr		;R14 = arr;
	MOV R15, R14		;R15 = arr;
	MOV R14, 1
	MOV [R15], R14		;*arr = 1;
	MOV R14, arr		;R14 = arr;
	MOV R13, 4
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R13
	ADD R14, RAX		;R14 = arr + 4;
	POP RAX
	POP RDX
	MOV R15, R14		;R15 = arr + 4;
	MOV R14, 2
	MOV [R15], R14		;*(arr + 4) = 2;
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
