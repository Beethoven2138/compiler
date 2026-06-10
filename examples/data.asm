section .data
	data dq 5
section .bss
	bss: resq 1
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 8
_scope_start1:
	MOV R15, 6
	MOV [data], R15
	MOV R14, data
	MOV [RBP-16], R14
	MOV R12, [RBP-16]
	MOV R13, [R12]
	MOV [bss], R13
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main