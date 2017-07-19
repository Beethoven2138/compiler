#ifndef ASM_H
#define ASM_H

#include <file.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

typedef char REGISTER;

/*enum
{
	RAX,
	RCX,
	RDX,
	RBX,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	R14,
	R15,
	FULL,
};*/

enum
{
	R15,
	R14,
	R13,
	R12,
	R11,
	R10,
	R9,
	R8,
	RBX,
	RDX,
	RCX,
	RAX,
};

enum
{
	TIMMEDIATE,
	TREGISTER,
};

typedef struct
{
	int type;
	int value;
} OPERAND;

//OPERAND operand;

bool reg_gen[8];

const char *registers[12];

void init_registers(char *file_name);

REGISTER reg_alloc(void);
void reg_free(REGISTER reg);

void MOV_R64R64(int dest, int src);
void MOV_R64I(int dest, int src);

void ADD_R64R64(int dest, int src);
void ADD_R64I(int dest, int src);

void SUB_R64R64(int dest, int src);
void SUB_R64I(int dest, int src);

void MUL_R64(int src);

void DIV_R64(int src);

void PUSH(int src);
void POP(int dest);

void JMP(char *routine);
void JZ(char *routine);//if 0
void JNZ(char *routine);//if !0
void JG(char *routine);//if greater than (signed)
void JL(char *routine);//if less than (signed)
void JA(char *routine);//if greater than (unsigned)
void JB(char *routine);//if less than (unsigned)
void JE(char *routine);//if ==
void JNE(char *routine);//if !=
void CMP(REGISTER op1, OPERAND op2);//Left to right
void CALL(char *routine);

void XOR_R64R64(REGISTER reg1, REGISTER reg2);
void XOR_R64I(REGISTER reg1, int immediate);

void AND_R64R64(REGISTER reg1, REGISTER reg2);

void OR_R64R64(REGISTER reg1, REGISTER reg2);

void write_to_file(void);

#endif
