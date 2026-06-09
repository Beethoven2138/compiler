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
	MOV [RBP-16], R15
	LEA R14, [RBP-16]
	MOV [RBP-24], R14
	MOV R13, [RBP-24]
	MOV R12, 6
	MOV [0+R13], R12
_scope_end1:
	MOV RAX, 60
	MOV RDI, 0
	syscall
_scope_end0:
