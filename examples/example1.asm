section .text
	global main
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
_scope_start1:
	MOV R15, 5
_scope_start2:
	XOR R14, R14
_scope_end2:
_loop0:
	MOV R13, R14
	MOV R12, R15
	CMP R13, R12
	JA _rif_0
	MOV R13, 1
	JMP _rend_0
_rif_0:
	XOR R13, R13
_rend_0:
	CMP R13, 1
	JNE _loop_end0
_scope_start3:
	MOV R12, 3
_scope_end3:
_scope_start4:
	MOV R11, 1
	ADD R14, R11
_scope_end4:
	JMP _loop0
_loop_end0:
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
