section .data
section .bss
section .text
	global _start
_scope_start0:
main:
	PUSH RBP
	MOV RBP, RSP
	PUSH RBX
	SUB RSP, 16
_scope_start1:
	MOV R15, 5
	MOV [RBP-16], R15	;a=5
	LEA R14, [RBP-16]
	MOV [RBP-24], R14	;ptr = &a
	LEA R11, [RBP-24]
	MOV R12, [R11]
	MOV R13, 6
	MOV [R12], R13
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
_start:
	CALL main
