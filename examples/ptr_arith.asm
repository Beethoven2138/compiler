section .data
section .bss
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 24
_scope_start1:
	MOV R15, 5
	MOV [RBP-16], R15	;local = 5;
	LEA R14, [RBP-16]	;R14 = &local;
	MOV [RBP-32], R14	;ptr1 = &local;
	MOV R11, [RBP-24]	;R11 = ptr;
	MOV R9, [RBP-32]	;R9 = ptr1;
	MOV R10, [R9]		;R10 = *ptr1;
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R10
	ADD R11, RAX		;R11 = ptr + *ptr1;
	POP RAX
	POP RDX
	MOV R12, R11		;R12 = ptr + *ptr1;
	MOV R13, [R12]		;R13 = *(ptr + *ptr1);
	MOV [RBP-16], R13	;local = *(ptr + *ptr1);
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
