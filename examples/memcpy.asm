section .data
section .bss
	arr1: resq 10
	arr2: resq 5
section .text
	global _start
_scope_start0:
memcpy64:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 8
_scope_start1:
_scope_start2:
	XOR R15, R15
	MOV [RBP-16], R15
_scope_end2:
_loop0:
	MOV R14, [RBP-16]
	MOV R13, [RBP+32]
	CMP R14, R13
	JAE _rif_0
	MOV R14, 1
	JMP _rend_0
_rif_0:
	XOR R14, R14
_rend_0:
	CMP R14, 1
	JNE _loop_end0
_scope_start3:
	MOV R12, [RBP+16]
	MOV R11, [RBP-16]
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R11
	ADD R12, RAX
	POP RAX
	POP RDX
	MOV R13, R12
	MOV R10, [RBP+24]
	MOV R9, [RBP-16]
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R9
	ADD R10, RAX
	POP RAX
	POP RDX
	MOV R11, R10
	MOV R12, [R11]
	MOV [R13], R12
_scope_end3:
_scope_start4:
	MOV R13, [RBP-16]
	MOV R12, 1
	ADD R13, R12
	MOV [RBP-16], R13
_scope_end4:
	JMP _loop0
_loop_end0:
	XOR R12, R12
	MOV RAX, R12
	ADD RSP, 8
	POP RBX
	POP RBP
	RET
_scope_end1:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 16
_scope_start5:
_scope_start6:
	XOR R15, R15
	MOV [RBP-16], R15
_scope_end6:
_loop1:
	MOV R12, [RBP-16]
	MOV R11, 5
	CMP R12, R11
	JAE _rif_1
	MOV R12, 1
	JMP _rend_1
_rif_1:
	XOR R12, R12
_rend_1:
	CMP R12, 1
	JNE _loop_end1
_scope_start7:
	MOV R10, arr2
	MOV R9, [RBP-16]
	PUSH RDX
	PUSH RAX
	MOV RAX, 8
	MUL R9
	ADD R10, RAX
	POP RAX
	POP RDX
	MOV R11, R10
	MOV R10, [RBP-16]
	MOV R9, 3
	PUSH RDX
	PUSH RAX
	MOV RAX, R10
	MUL R9
	MOV R10, RAX
	POP RAX
	POP RDX
	MOV [R11], R10
_scope_end7:
_scope_start8:
	MOV R11, [RBP-16]
	MOV R10, 1
	ADD R11, R10
	MOV [RBP-16], R11
_scope_end8:
	JMP _loop1
_loop_end1:
	MOV R9, 5
	PUSH R9
	MOV R9, arr2
	PUSH R9
	MOV R8, arr1
	MOV R9, R8
	PUSH R9
	CALL memcpy64
	ADD RSP, 24
	MOV R10, RAX
	MOV [RBP-16], R10
_scope_end5:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
