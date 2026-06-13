section .data
	data dq 1
section .bss
	arr: resq 5
section .text
	global _start
_scope_start0:
_scope_start1:
	MOV R15, 4
	MOV [RBP-16], R15	;local = 4;
	LEA R14, [RBP-16]	;R14 = &local;
	MOV R13, 7
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R13
	ADD R14, RAX		;R14 = &local + 7;
	POP RAX
	POP RDX
	MOV [RBP-24], R14	;ptr = &local + 7;
	MOV R12, [arr]		;R12 = arr;
	MOV R11, 1
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R11
	ADD R12, RAX		;R12 = arr + 1;
	POP RAX
	POP RDX
	MOV R13, R12		;R13 = arr + 1;
	MOV R10, [RBP-24]	;R10 = ptr;
	MOV R9, 7
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R9
	SUB R10, RAX		;R10 = ptr - 7;
	POP RAX
	POP RDX
	MOV R11, R10		;R11 = ptr - 7;
	MOV R12, [R11]		;R12 = *(ptr - 7);
	MOV [R13], R12		;*(arr + 1) = *(ptr - 7);
_scope_end1:
_scope_end0:
_start:
	CALL main
