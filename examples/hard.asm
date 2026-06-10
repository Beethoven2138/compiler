section .data
section .bss
	ptr: resq 1
	bss_var: resq 1
section .text
	global _start
_scope_start0:
test_func:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 0
_scope_start1:
	MOV R14, [RBP+16]
	MOV R15, [R14]
	MOV RAX, R15
	ADD RSP, 0
	POP RBX
	POP RBP
	RET
_scope_end1:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 8
_scope_start2:
	MOV R15, 3
	MOV [RBP-16], R15
	LEA R14, [RBP-16]
	MOV [ptr], R14
	MOV R11, ptr
	MOV R12, R11
	PUSH R12
	CALL test_func
	ADD RSP, 8
	MOV R13, RAX
	MOV [bss_var], R13
_scope_end2:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main