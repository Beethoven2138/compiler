section .data
section .bss
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 32
_scope_start1:
	MOV R15, 1
	MOV [RBP-16], R15
	XOR R14, R14
	MOV [RBP-24], R14
	XOR R13, R13
	MOV [RBP-32], R13
_scope_start2:
	XOR R12, R12
	MOV [RBP-40], R12
_scope_end2:
_loop0:
	MOV R11, [RBP-40]
	MOV R10, 10
	CMP R11, R10
	JA _rif_0
	MOV R11, 1
	JMP _rend_0
_rif_0:
	XOR R11, R11
_rend_0:
	CMP R11, 1
	JNE _loop_end0
_scope_start3:
	MOV R10, [RBP-40]
	MOV R9, 2
	CMP R10, R9
	JBE _rif_1
	MOV R10, 1
	JMP _rend_1
_rif_1:
	XOR R10, R10
_rend_1:
	CMP R10, 1
	JNE rcondelse_0
_scope_start4:
	MOV R10, [RBP-16]
	MOV R9, [RBP-24]
	ADD R10, R9
	MOV [RBP-32], R10
	MOV R9, [RBP-16]
	MOV [RBP-24], R9
	MOV R8, [RBP-32]
	MOV [RBP-16], R8
_scope_end4:
	JMP rcondend_0
rcondelse_0:
rcondend_0:
_scope_end3:
_scope_start5:
	MOV R10, [RBP-40]
	MOV RBX, 1
	ADD R10, RBX
	MOV [RBP-40], R10
_scope_end5:
	JMP _loop0
_loop_end0:
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
