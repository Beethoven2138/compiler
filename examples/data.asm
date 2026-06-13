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
	MOV [data], R15		;data = 6;
	MOV R14, data		;R14 = &data;
	MOV [RBP-16], R14	;ptr = &data;
	MOV R12, [RBP-16]	;R12 = ptr;
	MOV R13, [R12]		;R13 = *ptr;
	MOV [bss], R13		;bss = *ptr;
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
