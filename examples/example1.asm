section .data
section .bss
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 24
_scope_start1:
	MOV R15, 5
	MOV [RBP-16], R15
_scope_start2:
	XOR R14, R14
	MOV [RBP-24], R14
_scope_end2:
_loop0:
	MOV R13, [RBP-24]
	MOV R12, [RBP-16]
	CMP R13, R12
	JAE _rif_0
	MOV R13, 1
	JMP _rend_0
_rif_0:
	XOR R13, R13
_rend_0:
	CMP R13, 1
	JNE _loop_end0
_scope_start3:
	MOV R12, 3
	MOV R11, [RBP-24]
	MOV RAX, R12
	MUL R11
	MOV R12, RAX
	MOV [RBP-32], R12
_scope_end3:
_scope_start4:
	MOV R11, [RBP-24]
	MOV R10, 1
	ADD R11, R10
	MOV [RBP-24], R11
_scope_end4:
	JMP _loop0
_loop_end0:
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
