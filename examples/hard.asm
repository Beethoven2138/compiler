section .data
	data_var dq 5
section .bss
	ptr: resq 1
section .text
	global _start
_scope_start0:
test_func:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 0
_scope_start1:
	MOV R13, [RBP+16]	;R13 = arg;
	MOV R12, 2
	PUSH RAX
	MOV RAX, 8
	MUL R12
	SUB R13, RAX		;R13 = arg - 2;
	POP RAX
	MOV R14, R13
	MOV R15, [R14]		;R15 = *(arg - 2);
	MOV RAX, R15		;RAX = *(arg - 2);
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
	MOV [RBP-16], R15	;local = 3;
	LEA R14, [RBP-16]
	MOV [ptr], R14		;ptr = &local;
	MOV R11, [ptr]		;R11 = ptr;
	MOV R10, 2
	PUSH RAX
	MOV RAX, 8
	MUL R10
	ADD R11, RAX		;R11 = ptr + 2;
	POP RAX
	MOV R12, R11		;R12 = ptr + 2;
	PUSH R12
	CALL test_func		;test_func(ptr + 2);
	ADD RSP, 8
	MOV R13, RAX
	MOV [data_var], R13	;data_var = test_func(ptr + 2);
_scope_end2:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
