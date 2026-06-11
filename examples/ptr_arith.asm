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
	MOV R15, 5
	MOV [RBP-16], R15
	LEA R14, [RBP-16]
	MOV R13, 3
	PUSH RAX
	MOV RAX, 8
	MUL R13
	ADD R14, RAX
	POP RAX
	MOV [RBP-24], R14
	MOV R11, [RBP-24]
	MOV R10, 3
	PUSH RAX
	MOV RAX, 8
	MUL R10
	SUB R11, RAX
	POP RAX
	MOV R12, R11
	MOV R13, [R12]
	MOV [RBP-16], R13
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main