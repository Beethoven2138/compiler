#include <asm.h>



File *fout;

void init_registers(char *file_name)
{
	/*registers[0] = "RAX";
	registers[1] = "RCX";
	registers[2] = "RDX";
	registers[3] = "RBX";
	registers[4] = "R8";
	registers[5] = "R9";
	registers[6] = "R10";
	registers[7] = "R11";
	registers[8] = "R12";
	registers[9] = "R13";
	registers[10] = "R14";
	registers[11] = "R15";*/

	registers[0] = "R15";
	registers[1] = "R14";
	registers[2] = "R13";
	registers[3] = "R12";
	registers[4] = "R11";
	registers[5] = "R10";
	registers[6] = "R9";
	registers[7] = "R8";
	registers[8] = "RBX";
	registers[9] = "RDX";
	registers[10] = "RCX";
	registers[11] = "RAX";

	spec_registers[0] = "RSP";
	spec_registers[1] = "RBP";
	spec_registers[2] = "RDI";
	spec_registers[3] = "RSI";

	fout = (File*)malloc(sizeof(File));

	fout->file = fopen(file_name, "w");
	assert(fout->file);

	fout->name = file_name;
	fout->text = (char*)malloc(sizeof(char) * 2);
	fout->text[1] = 0;
	fout->index = 0;
	fout->str_len = 1;

	write_strn("section .text\n", 14);
	writec(9);
	write_strn("global main\n", 12);
}

REGISTER reg_alloc(void)
{
	for (int i = 0; i < 11; i++)
	{
		if (reg_gen[i] == 0)
		{
			reg_gen[i] = 1;
			return i;
		}
	}
	assert(0);
}

void reg_free(REGISTER reg)
{
	reg_gen[reg] = 0;
}

void MOV_R64R64(int dest, int src)
{
	writec(9);
	write_strn("MOV ", 4);
	write_str(registers[dest]);
	write_strn(", ", 2);
	write_str(registers[src]);
	writec('\n');
}

void MOV_R64I(int dest, int src)
{
	if (src == 0)
	{
		XOR_R64R64(dest, dest);
	}
	else
	{
		writec(9);
		write_strn("MOV ", 4);
		write_str(registers[dest]);
		write_strn(", ", 2);
		char tmp[100];
		sprintf(tmp, "%d", src);
		write_str(tmp);
		writec('\n');
	}
}

void MOV_R64OFF(int dest, int off)
{
	writec(9);
	write_strn("MOV ", 4);
	write_str(registers[dest]);
	write_strn(", ", 2);
	char tmp[100];
	sprintf(tmp, "[RBP+%d]", off);
	write_str(tmp);
	writec('\n');
}

void MOV_OFFI(int off, int src)
{
	writec(9);
	write_strn("MOV ", 4);
	char tmp[100];
	sprintf(tmp, "[RBP+%d], ", off);
	write_str(tmp);
	sprintf(tmp, "%d", src);
	write_str(tmp);
	writec('\n');
}

void MOV_OFFR64(int off, int src)
{
	writec(9);
	write_strn("MOV ", 4);
	char tmp[100];
	sprintf(tmp, "[RBP+%d], ", off);
	write_str(tmp);
	write_str(registers[src]);
	writec('\n');
}

void ADD_R64R64(int dest, int src)
{
	writec(9);
	write_strn("ADD ", 4);
	write_str(registers[dest]);
	write_strn(", ", 2);
	write_str(registers[src]);
	writec('\n');
}

void ADD_R64I(int dest, int src)
{
	writec(9);
	write_strn("ADD ", 4);
	write_str(registers[dest]);
	write_strn(", ", 2);
	char tmp[100];
	sprintf(tmp, "%d", src);
	write_str(tmp);
	writec('\n');
}


void SUB_R64R64(int dest, int src)
{
	writec(9);
	write_strn("SUB ", 4);
	write_str(registers[dest]);
	write_strn(", ", 2);
	write_str(registers[src]);
	writec('\n');
}

void SUB_R64I(int dest, int src)
{
	writec(9);
	write_strn("SUB ", 4);
	write_str(registers[dest]);
	write_strn(", ", 2);
	char tmp[100];
	sprintf(tmp, "%d", src);
	write_str(tmp);
	writec('\n');
}


//RAX always first operand
void MUL_R64(int src)
{
	assert(src != RAX);
	writec(9);
	write_strn("MUL ", 4);
	write_str(registers[src]);
	writec('\n');
}

void DIV_R64(int src)
{
	assert(src != RAX);
	writec(9);
	write_strn("DIV ", 4);
	write_str(registers[src]);
	writec('\n');
}

void PUSH(int src)
{
	writec(9);
	write_strn("PUSH ", 5);
	write_str(registers[src]);
	writec('\n');
}

void POP(int dest)
{
	writec(9);
	write_strn("POP ", 4);
	write_str(registers[dest]);
	writec('\n');
}

void JMP(char *routine)
{
	writec(9);
	write_strn("JMP ", 4);
	write_str(routine);
	writec('\n');
}

//Jump if 0
void JZ(char *routine)
{
	writec(9);
	write_strn("JZ ", 3);
	write_str(routine);
	writec('\n');
}

//Jump if !0
void JNZ(char *routine)
{
	writec(9);
	write_strn("JNZ ", 4);
	write_str(routine);
	writec('\n');
}

//Jump if greater than (signed)
void JG(char *routine)
{
	writec(9);
	write_strn("JG ", 3);
	write_str(routine);
	writec('\n');
}

//Jump if less than (signed)
void JL(char *routine)
{
	writec(9);
	write_strn("JL ", 3);
	write_str(routine);
	writec('\n');
}

//Jump if greater than (unsigned)
void JA(char *routine)
{
	writec(9);
	write_strn("JA ", 3);
	write_str(routine);
	writec('\n');
}

//Jump if less than (unsigned)
void JB(char *routine)
{
	writec(9);
	write_strn("JB ", 3);
	write_str(routine);
	writec('\n');
}
//Jump if ==
void JE(char *routine)
{
	writec(9);
	write_strn("JE ", 3);
	write_str(routine);
	writec('\n');
}

//Jump if !=
void JNE(char *routine)
{
	writec(9);
	write_strn("JNE ", 4);
	write_str(routine);
	writec('\n');
}

//op1 must be a register
void CMP(REGISTER op1, OPERAND op2)
{
	writec(9);
	write_strn("CMP ", 4);
	write_str(registers[op1]);
	write_strn(", ", 2);
	if (op2.type == TREGISTER)
		write_str(registers[op2.value]);
	else if (op2.type == TIMMEDIATE)
	{
		char tmp[100];
		sprintf(tmp, "%d", op2.value);
		write_str(tmp);
	}
	writec('\n');
}

void CALL(char *routine)
{
	writec(9);
	write_strn("CALL ", 5);
	write_str(routine);
	writec('\n');
}


void XOR_R64R64(REGISTER reg1, REGISTER reg2)
{
	writec(9);
	write_strn("XOR ", 4);
	write_str(registers[reg1]);
	write_strn(", ", 2);
	write_str(registers[reg2]);
	writec('\n');
}

void XOR_R64I(REGISTER reg, int immediate)
{
	writec(9);
	write_strn("XOR ", 4);
	write_str(registers[reg]);
	write_strn(", ", 2);
	char tmp[100];
	sprintf(tmp, "%d", immediate);
	write_str(tmp);
	writec('\n');
}

void AND_R64R64(REGISTER reg1, REGISTER reg2)
{
	writec(9);
	write_strn("AND ", 4);
	write_str(registers[reg1]);
	write_strn(", ", 2);
	write_str(registers[reg2]);
	writec('\n');
}

void OR_R64R64(REGISTER reg1, REGISTER reg2)
{
	writec(9);
	write_strn("OR ", 3);
	write_str(registers[reg1]);
	write_strn(", ", 2);
	write_str(registers[reg2]);
	writec('\n');
}

void INC(REGISTER dest)
{
	writec(9);
	write_strn("INC ", 4);
	write_str(registers[dest]);
	writec('\n');
}

void DEC(REGISTER dest)
{
	writec(9);
	write_strn("DEC ", 4);
	write_str(registers[dest]);
	writec('\n');
}

/*
The function calling conventions that this compiler uses are
the ones stated in the book "Modern x86 Assembly Language Programming".
Function arguments can be accessed by:
[RBP + offset];
Start at [RBP + 8]; since the first 8 bytes hold the return address.
*/

void func_prolog(void)
{
	//These registers must be preserved
	writec(9);
	write_strn("PUSH RBP\n", 9);
	writec(9);
	write_strn("MOV RBP, RSP\n", 13);
	writec(9);
	write_strn("PUSH RBX\n", 9);
}

void func_epilog(void)
{
	write_strn("POP RBX\n", 8);
	writec(9);
	write_strn("POP RBP\n", 8);
	writec(9);
	write_strn("RET\n", 4);
}

void EXIT(void)
{
	writec(9);
	write_strn("MOV RAX, 60\n", 12);
	writec(9);
	write_strn("MOV RDI, 0\n", 11);
	writec(9);
	write_strn("syscall\n", 8);
}

void write_to_file(void)
{
	fputs(fout->text, fout->file);
}
