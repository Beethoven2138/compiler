section .data
section .bss
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 8
_scope_start1:
	XOR R15, R15
	MOV [RBP-16], R15	;i = 0;
_loop0:
	MOV R14, [RBP-16]	;R14 = i;
	MOV R13, 10		;R13 = 10;
	CMP R14, R13		;CMP i, 10
	JAE _rif_0		;if i >= 10
	MOV R14, 1
	JMP _rend_0
_rif_0:				;if i >= 10
	XOR R14, R14		;R14 = 0;
_rend_0:
	CMP R14, 1
	JNE _loop_end0		;if i >= 10
_scope_start2:
	MOV R13, [RBP-16]
	MOV R12, 1
	ADD R13, R12
	MOV [RBP-16], R13	;i = i + 1
_scope_end2:
	JMP _loop0
_loop_end0:
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
