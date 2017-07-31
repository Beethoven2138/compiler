#ifndef ASM_H
#define ASM_H

#include <file.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <file.h>
#include <lex.h>

typedef char REGISTER;


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
	RSP,
	RBP,
	RDI,
	RSI,
};

enum
{
	TIMMEDIATE,
	TREGISTER,
	TOFFSET,
	TSEG_DATA,
	TSEG_BSS,
};

enum
{
	BYTE = 1,
	WORD = 2,
	DWORD = 4,
	QWORD = 8,
};

typedef struct OPERAND
{
	/*says if its bss, data, register etc*/
	int type;

	/*8-bit, 16-bit etc*/
	int data_type;

	/*holds the register being used
	  if no register is used, value = -1
	*/
	char *id;

	union
	{
		int value;

		struct
		{
			int off_type;
			int off;
			char* base_ptr;
		};
	};
} OPERAND;




//OPERAND operand;

bool reg_gen[8];

const char *registers[12];
const char *spec_registers[4];

const char *registers32[12];
const char *registers16[12];
const char *registers8[12];

void init_registers(char *file_name);

void init_data(BUFFER *text);
void init_bss(BUFFER *text);

REGISTER reg_alloc(void);
void reg_free(REGISTER reg);

void MOV_R64R64(int dest, int src, int size);
void MOV_R64I(int dest, int src, int size);

void MOV_R64OFF(int dest, int off, int off_type, char *base_ptr, int size);
void MOV_OFFR64(int off, int off_type, char *base_ptr, int src, int size);
void MOV_OFFI(int off, int off_type, char *base_ptr, int src, int size);

void MOV_DR64(char *str, int src, int size);
void MOV_R64D(int dest, char *src, int size);
void MOV_DI(char *str, int src, int size);

void MOV_R64ADR(int dest, char *str, int size);

void LEA(int dest, int off, int off_type, char *base_ptr, int size);

void ADD_R64R64(int dest, int src, int size1, int size2);
void ADD_R64I(int dest, int src, int size);

void SUB_R64R64(int dest, int src, int size1, int size2);
void SUB_R64I(int dest, int src, int size);

void MUL_R64(int src, int size);

void DIV_R64(int src, int size);

void PUSH(int src, int size);
void POP(int dest, int size);

void JMP(char *routine);
void JZ(char *routine);//if 0
void JNZ(char *routine);//if !0
void JG(char *routine);//if greater than (signed)
void JL(char *routine);//if less than (signed)
void JA(char *routine);//if greater than (unsigned)
void JB(char *routine);//if less than (unsigned)
void JE(char *routine);//if ==
void JNE(char *routine);//if !=
void CMP(REGISTER op1, OPERAND op2, int size);//Left to right
void CALL(char *routine);

void XOR_R64R64(REGISTER reg1, REGISTER reg2, int size1,  int size2);
void XOR_R64I(REGISTER reg1, int immediate, int size);

void AND_R64R64(REGISTER reg1, REGISTER reg2, int size1, int size2);

void OR_R64R64(REGISTER reg1, REGISTER reg2, int size1, int size2);

void INC(REGISTER dest, int size);
void DEC(REGISTER dest, int size);

//used at the start of a function
void func_prolog(void);
//used at the end of a function
void func_epilog(void);


void EXIT(void);

void add_data(char *name, int type, int value);
void add_bss(char *name, int type, int size);


void write_to_file(void);


int sizeof_data(int data_type);

#endif
