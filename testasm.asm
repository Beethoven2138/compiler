section .data
section .bss
section .text
	global _start
_scope_start0:
efunc:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
_scope_start1:
	MOV R15, [RBP+16]
	MOV [RBP-8], R15
	MOV RAX, [RBP-8]
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
	MOV [RBP-8], R15
	MOV R14, [RBP-8]
	MOV R13, 2
	MOV RAX, R14
	MUL R13
	MOV R14, RAX
	MOV [RBP-16], R14
	MOV R13, [RBP-8]
	MOV R12, [RBP-16]
	CMP R13, R12
	JNE _rif_0
	MOV R13, 1
	JMP _rend_0
_rif_0:
	XOR R13, R13
_rend_0:
	MOV R12, [RBP-8]
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
	MOV R13, [RBP-16]
	MOV R12, 34
	SUB R13, R12
	MOV [RBP-24], R13
_scope_end3:
	JMP rcondend_0
rcondelse_0:
_scope_start4:
	MOV R15, 4556
	MOV [RBP-8], R15
_scope_end4:
rcondend_0:
	CALL efunc
	POP R15
	MOV R13, RAX
	MOV [RBP-16], R13
_loop0:
	MOV R12, [RBP-8]
	MOV R11, 45
	CMP R12, R11
	JE _rif_2
	MOV R12, 1
	JMP _rend_2
_rif_2:
	XOR R12, R12
_rend_2:
	CMP R12, 1
	JNE _loop_end0
_scope_start5:
	MOV R11, 45
	MOV [RBP-8], R11
_scope_end5:
	JMP _loop0
_loop_end0:
_scope_end2:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
