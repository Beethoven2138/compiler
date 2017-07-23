section .text
	global main
_scope_start0:
efunc:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
_scope_start1:
	MOV R15, [RBP+8]
	MOV RAX, R15
	POP RBX
	POP RBP
	RET
_scope_end1:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
_scope_start2:
	MOV R15, 45
	MOV R14, R15
	MOV R13, 2
	MOV RAX, R14
	MUL R13
	MOV R14, RAX
	MOV R13, R15
	MOV R12, R14
	CMP R13, R12
	JNE _rif_0
	MOV R13, 1
	JMP _rend_0
_rif_0:
	XOR R13, R13
_rend_0:
	MOV R12, R15
	MOV R11, 5
	CMP R12, R11
	JNE _rif_1
	MOV R12, 1
	JMP _rend_1
_rif_1:
	XOR R12, R12
_rend_1:
	AND R13, R12
	JNE _rlog_0
	MOV R13, 1
	JMP _rendlog_0
_rlog_0:
	XOR R13, R13
_rendlog_0:
	CMP R13, 1
	JNE rcondelse_0
_scope_start3:
	MOV R13, R14
	MOV R12, 34
	SUB R13, R12
_scope_end3:
	JMP rcondend_0
rcondelse_0:
_scope_start4:
	MOV R15, 4556
_scope_end4:
rcondend_0:
	PUSH R14
	CALL efunc
	MOV R14, RAX
_loop0:
	MOV R13, R15
	MOV R12, 45
	CMP R13, R12
	JE _rif_2
	MOV R13, 1
	JMP _rend_2
_rif_2:
	XOR R13, R13
_rend_2:
	CMP R13, 1
	JNE _loop_end0
_scope_start5:
	MOV R15, 45
_scope_end5:
	JMP _loop0
_loop_end0:
_scope_end2:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
