section .data
section .bss
	arr: resq 5
section .text
	global _start
_scope_start0:
_scope_start1:
	MOV R15, 4
	MOV [RBP-16], R15
	LEA R14, [RBP-16]
	MOV R13, 7
	PUSH RAX
	MOV RAX, 8
	MUL R13
	ADD R14, RAX
	POP RAX
	MOV [RBP-24], R14
	MOV R12, [arr]
	MOV R11, 2
	PUSH RAX
	MOV RAX, 8
	MUL R11
	ADD R12, RAX
	POP RAX
	MOV R13, R12
	MOV R10, [RBP-24]
	MOV R9, 7
	PUSH RAX
	MOV RAX, 8
	MUL R9
	SUB R10, RAX
	POP RAX
	MOV R11, R10
	MOV R12, R11
	MOV [R13], R12
_scope_end1:
_scope_end0:
_start:
	CALL main