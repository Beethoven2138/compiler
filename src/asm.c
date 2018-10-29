#include <asm.h>



File *fout;

void init_registers(char *file_name)
{
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

	registers32[0] = "R15D";
	registers32[1] = "R14D";
	registers32[2] = "R13D";
	registers32[3] = "R12D";
	registers32[4] = "R11D";
	registers32[5] = "R10D";
	registers32[6] = "R9D";
	registers32[7] = "R8D";
	registers32[8] = "EBX";
	registers32[9] = "EDX";
	registers32[10] = "ECX";
	registers32[11] = "EAX";

	registers16[0] = "R15W";
	registers16[1] = "R14W";
	registers16[2] = "R13W";
	registers16[3] = "R12W";
	registers16[4] = "R11W";
	registers16[5] = "R10W";
	registers16[6] = "R9W";
	registers16[7] = "R8W";
	registers16[8] = "EBX";
	registers16[9] = "DX";
	registers16[10] = "CX";
	registers16[11] = "AX";

	registers8[0] = "R15B";
	registers8[1] = "R14B";
	registers8[2] = "R13B";
	registers8[3] = "R12B";
	registers8[4] = "R11B";
	registers8[5] = "R10B";
	registers8[6] = "R9B";
	registers8[7] = "R8B";
	registers8[8] = "BL";
	registers8[9] = "DL";
	registers8[10] = "CL";
	registers8[11] = "AL";

	spec_registers[0] = "RSP";
	spec_registers[1] = "RBP";
	spec_registers[2] = "RDI";
	spec_registers[3] = "RSI";

	fout = (File*)malloc(sizeof(File));

	fout->file = fopen(file_name, "w");
	assert(fout->file);

	fout->buff = (BUFFER*)malloc(sizeof(BUFFER) * 3);

	fout->name = file_name;
	fout->buff[0].text = (char*)malloc(sizeof(char));
	fout->buff[0].text[0] = 0;
	fout->buff[0].index = 0;
	fout->buff[1].text = (char*)malloc(sizeof(char));
	fout->buff[1].text[0] = 0;
	fout->buff[1].index = 0;
	fout->buff[2].text = (char*)malloc(2);
	fout->buff[2].text[0] = 0;
	fout->buff[2].index = 0;
	fout->buff[2].str_len = 1;

	write_strn("section .text\n", 14, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("global _start\n", 14, SECT_CODE);

	write_strn("section .data\n", 14, SECT_DATA);

	write_strn("section .bss\n", 13, SECT_BSS);
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

void MOV_R64R64(int dest, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}
	write_strn(", ", 2, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}
	writec('\n', SECT_CODE);
}

void MOV_R64I(int dest, int src, int size)
{
	if (src == 0)
	{
		XOR_R64R64(dest, dest, size, size);
	}
	else
	{
		writec(9, SECT_CODE);
		write_strn("MOV ", 4, SECT_CODE);
		switch (size)
		{
		case QWORD:
			write_str(registers[dest], SECT_CODE);
			break;
		case DWORD:
			write_str(registers32[dest], SECT_CODE);
			break;
		case WORD:
			write_str(registers16[dest], SECT_CODE);
			break;
		case BYTE:
			write_str(registers8[dest], SECT_CODE);
			break;
		}

		write_strn(", ", 2, SECT_CODE);
		char tmp[100];
		sprintf(tmp, "%d", src);
		write_str(tmp, SECT_CODE);
		writec('\n', SECT_CODE);
	}
}

void MOV_R64OFF(int dest, int off, int off_type, char *base_ptr, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	char tmp[100];
	if (off_type == TIMMEDIATE)
	{
		if (off > 0)
			sprintf(tmp, "[%s+%d]", base_ptr, off);
		else
			sprintf(tmp, "[%s]", base_ptr);
	}
	else if (off_type == TREGISTER)
	{
		switch (size)
		{
		case QWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers[off]);
			break;
		case DWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers32[off]);
			break;
		case WORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers16[off]);
			break;
		case BYTE:
			sprintf(tmp, "[%s+%s]", base_ptr, registers8[off]);
			break;
		}
	}
	write_str(tmp, SECT_CODE);
	writec('\n', SECT_CODE);
}

void MOV_OFFR64(int off, int off_type, char *base_ptr, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	char tmp[100];
	if (off_type == TIMMEDIATE)
	{
		if (off > 0)
			sprintf(tmp, "[%s+%d]", base_ptr, off);
		else
			sprintf(tmp, "[%s]", base_ptr, off);
	}
	else if (off_type == TREGISTER)
	{
		switch (size)
		{
		case QWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers[off]);
			break;
		case DWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers32[off]);
			break;
		case WORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers16[off]);
			break;
		case BYTE:
			sprintf(tmp, "[%s+%s]", base_ptr, registers8[off]);
			break;
		}
	}
	write_str(tmp, SECT_CODE);
	write_strn(", ", 2, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}
	writec('\n', SECT_CODE);
}

void MOV_OFFI(int off, int off_type, char *base_ptr, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	char tmp[100];
	if (off_type == TIMMEDIATE)
		sprintf(tmp, "[%s+%d]", base_ptr, off);
	else if (off_type == TREGISTER)
	{
		switch (size)
		{
		case QWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers[off]);
			break;
		case DWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers32[off]);
			break;
		case WORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers16[off]);
			break;
		case BYTE:
			sprintf(tmp, "[%s+%s]", base_ptr, registers8[off]);
			break;
		}
	}

	write_str(tmp, SECT_CODE);

	write_strn(", ", 2, SECT_CODE);

	switch (size)
	{
	case BYTE:
	{
		write_strn("byte ", 5, SECT_CODE);
		break;
	}
	case WORD:
	{
		write_strn("word ", 5, SECT_CODE);
		break;
	}
	case DWORD:
	{
		write_strn("dword ", 6, SECT_CODE);
		break;
	}
	case QWORD:
	{
		write_strn("qword ", 6, SECT_CODE);
		break;
	}
	}

	sprintf(tmp, "%d", src);
	write_str(tmp, SECT_CODE);
	writec('\n', SECT_CODE);
}

void MOV_R64ADR(int dest, char *str, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}
	write_strn(", ", 2, SECT_CODE);
	write_str(str, SECT_CODE);
	writec('\n',  SECT_CODE);
}

void MOV_DR64(char *str, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	writec('[', SECT_CODE);
	write_str(str, SECT_CODE);
	write_strn("], ", 3, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void MOV_R64D(int dest, char *str, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	write_strn(", [", 3, SECT_CODE);
	write_str(str, SECT_CODE);
	write_strn("]\n", 2, SECT_CODE);
}

void MOV_DI(char *str, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("MOV ", 4, SECT_CODE);
	writec('[', SECT_CODE);
	write_str(str, SECT_CODE);
	write_strn("], ", 3, SECT_CODE);
	switch (size)
	{
	case BYTE:
	{
		write_strn("byte ", 5, SECT_CODE);
		break;
	}
	case WORD:
	{
		write_strn("word ", 5, SECT_CODE);
		break;
	}
	case DWORD:
	{
		write_strn("dword ", 6, SECT_CODE);
		break;
	}
	case QWORD:
	{
		write_strn("qword ", 6, SECT_CODE);
		break;
	}
	}
	char tmp[100];
	sprintf(tmp, "%d\n", src);
	write_str(tmp, SECT_CODE);
}


void LEA(int dest, int off, int off_type, char *base_ptr, int size)
{
	writec(9, SECT_CODE);
	write_strn("LEA ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}
	write_strn(", ", 2, SECT_CODE);
	char tmp[100];
	if (off_type == TIMMEDIATE)
		sprintf(tmp, "[%s+%d]", base_ptr, off);
	else if (off_type == TREGISTER)
	{
		switch (size)
		{
		case QWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers[off]);
			break;
		case DWORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers32[off]);
			break;
		case WORD:
			sprintf(tmp, "[%s+%s]", base_ptr, registers16[off]);
			break;
		case BYTE:
			sprintf(tmp, "[%s+%s]", base_ptr, registers8[off]);
			break;
		}
	}
	write_str(tmp, SECT_CODE);
	writec('\n', SECT_CODE);
}


void ADD_R64R64(int dest, int src, int size1, int size2)
{
	writec(9, SECT_CODE);
	write_strn("ADD ", 4, SECT_CODE);
	switch (size1)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	switch (size2)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void ADD_R64I(int dest, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("ADD ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	char tmp[100];
	sprintf(tmp, "%d", src);
	write_str(tmp, SECT_CODE);
	writec('\n', SECT_CODE);
}


void SUB_R64R64(int dest, int src, int size1, int size2)
{
	writec(9, SECT_CODE);
	write_strn("SUB ", 4, SECT_CODE);
	switch (size1)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	switch (size2)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void SUB_R64I(int dest, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("SUB ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	char tmp[100];
	sprintf(tmp, "%d", src);
	write_str(tmp, SECT_CODE);
	writec('\n', SECT_CODE);
}


//RAX always first operand
void MUL_R64(int src, int size)
{
	assert(src != RAX);
	writec(9, SECT_CODE);
	write_strn("MUL ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void DIV_R64(int src, int size)
{
	assert(src != RAX);
	writec(9, SECT_CODE);
	write_strn("DIV ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void PUSH(int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("PUSH ", 5, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[src], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[src], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[src], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[src], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void POP(int dest, int size)
{
	writec(9, SECT_CODE);
	write_strn("POP ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void JMP(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JMP ", 4, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if 0
void JZ(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JZ ", 3, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if !0
void JNZ(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JNZ ", 4, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if greater than (signed)
void JG(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JG ", 3, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if less than (signed)
void JL(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JL ", 3, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if greater than (unsigned)
void JA(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JA ", 3, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if less than (unsigned)
void JB(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JB ", 3, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}
//Jump if ==
void JE(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JE ", 3, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//Jump if !=
void JNE(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("JNE ", 4, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}

//op1 must be a register
void CMP(REGISTER op1, OPERAND op2, int size)
{
	writec(9, SECT_CODE);
	write_strn("CMP ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[op1], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[op1], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[op1], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[op1], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	if (op2.type == TREGISTER)
	{
		//write_str(registers[op2.value], SECT_CODE);
		switch (sizeof_data(op2.data_type))
		{
		case QWORD:
			write_str(registers[op2.value], SECT_CODE);
			break;
		case DWORD:
			write_str(registers32[op2.value], SECT_CODE);
			break;
		case WORD:
			write_str(registers16[op2.value], SECT_CODE);
			break;
		case BYTE:
			write_str(registers8[op2.value], SECT_CODE);
			break;
		}
	}
	else if (op2.type == TIMMEDIATE)
	{
		char tmp[100];
		sprintf(tmp, "%d", op2.value);
		write_str(tmp, SECT_CODE);
	}
	writec('\n', SECT_CODE);
}

void CALL(char *routine)
{
	writec(9, SECT_CODE);
	write_strn("CALL ", 5, SECT_CODE);
	write_str(routine, SECT_CODE);
	writec('\n', SECT_CODE);
}


void XOR_R64R64(REGISTER reg1, REGISTER reg2, int size1, int size2)
{
	writec(9, SECT_CODE);
	write_strn("XOR ", 4, SECT_CODE);
	//write_str(registers[reg1], SECT_CODE);
	switch (size1)
	{
	case QWORD:
		write_str(registers[reg1], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg1], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg1], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg1], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	//write_str(registers[reg2], SECT_CODE);
	switch (size2)
	{
	case QWORD:
		write_str(registers[reg2], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg2], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg2], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg2], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void XOR_R64I(REGISTER reg, int immediate, int size)
{
	writec(9, SECT_CODE);
	write_strn("XOR ", 4, SECT_CODE);
	//write_str(registers[reg], SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[reg], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	char tmp[100];
	sprintf(tmp, "%d", immediate);
	write_str(tmp, SECT_CODE);
	writec('\n', SECT_CODE);
}

void AND_R64R64(REGISTER reg1, REGISTER reg2, int size1, int size2)
{
	writec(9, SECT_CODE);
	write_strn("AND ", 4, SECT_CODE);
	//write_str(registers[reg1], SECT_CODE);
	switch (size1)
	{
	case QWORD:
		write_str(registers[reg1], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg1], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg1], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg1], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	//write_str(registers[reg2], SECT_CODE);
	switch (size2)
	{
	case QWORD:
		write_str(registers[reg2], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg2], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg2], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg2], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void OR_R64R64(REGISTER reg1, REGISTER reg2, int size1, int size2)
{
	writec(9, SECT_CODE);
	write_strn("OR ", 3, SECT_CODE);
	//write_str(registers[reg1], SECT_CODE);
	switch (size1)
	{
	case QWORD:
		write_str(registers[reg1], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg1], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg1], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg1], SECT_CODE);
		break;
	}

	write_strn(", ", 2, SECT_CODE);
	//write_str(registers[reg2], SECT_CODE);
	switch (size2)
	{
	case QWORD:
		write_str(registers[reg2], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[reg2], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[reg2], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[reg2], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void INC(REGISTER dest, int size)
{
	writec(9, SECT_CODE);
	write_strn("INC ", 4, SECT_CODE);
	//write_str(registers[dest], SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void DEC(REGISTER dest, int size)
{
	writec(9, SECT_CODE);
	write_strn("DEC ", 4, SECT_CODE);
	//write_str(registers[dest], SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}

	writec('\n', SECT_CODE);
}

void SHL(REGISTER dest, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("SHL ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}
	char tmp[100];
	sprintf(tmp, ", %d\n", src);
	write_str(tmp, SECT_CODE);
}

void SHR(REGISTER dest, int src, int size)
{
	writec(9, SECT_CODE);
	write_strn("SHR ", 4, SECT_CODE);
	switch (size)
	{
	case QWORD:
		write_str(registers[dest], SECT_CODE);
		break;
	case DWORD:
		write_str(registers32[dest], SECT_CODE);
		break;
	case WORD:
		write_str(registers16[dest], SECT_CODE);
		break;
	case BYTE:
		write_str(registers8[dest], SECT_CODE);
		break;
	}
	char tmp[100];
	sprintf(tmp, ", %d/n", src);
	write_str(tmp, SECT_CODE);
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
	writec(9, SECT_CODE);
	write_strn("PUSH RBP\n", 9, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("MOV RBP, RSP\n", 13, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("PUSH RBX\n", 9, SECT_CODE);
}

void func_epilog(void)
{
	write_strn("POP RBX\n", 8, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("POP RBP\n", 8, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("RET\n", 4, SECT_CODE);
}

void EXIT(void)
{
	writec(9, SECT_CODE);
	write_strn("MOV RAX, 60\n", 12, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("MOV RDI, 0\n", 11, SECT_CODE);
	writec(9, SECT_CODE);
	write_strn("syscall\n", 8, SECT_CODE);
}

void add_data(char *name, int type, int value, bool extern_link)
{
	writec(9, SECT_DATA);
	if (extern_link)
		write_strn("extern ", 7, SECT_DATA);
	write_str(name, SECT_DATA);
	write_strn(" d", 2, SECT_DATA);
	switch (sizeof_data(type))
	{
	case BYTE:
		writec('b', SECT_DATA);
		break;
	case WORD:
		writec('w', SECT_DATA);
		break;
	case DWORD:
		writec('d', SECT_DATA);
		break;
	case QWORD:
		writec('q', SECT_DATA);
		break;
	}

	char tmp[100];
	sprintf(tmp, " %d\n", value);
	write_str(tmp, SECT_DATA);
}

void add_bss(char *name, int type, int size, bool extern_link)
{
	writec(9, SECT_BSS);
	if (extern_link)
		write_strn("extern ", 7, SECT_BSS);
	write_str(name, SECT_BSS);
	writec(':', SECT_BSS);
	write_strn(" res", 4, SECT_BSS);
	switch (sizeof_data(type))
	{
	case BYTE:
	{
		writec('b', SECT_BSS);
		break;
	}
	case WORD:
	{
		writec('w', SECT_BSS);
		break;
	}
	case DWORD:
	{
		writec('d', SECT_BSS);
		break;
	}
	case QWORD:
	{
		writec('q', SECT_BSS);
		break;
	}
	}

	char tmp[100];
	sprintf(tmp, " %d\n", size);
	write_str(tmp, SECT_BSS);
}

void write_to_file(void)
{
	//fputs(fout->buff[2].text, fout->file);
	fwrite(fout->buff[SECT_DATA].text, 1, fout->buff[SECT_DATA].index, fout->file);
	fwrite(fout->buff[SECT_BSS].text, 1, fout->buff[SECT_BSS].index, fout->file);
	fwrite(fout->buff[SECT_CODE].text, 1, fout->buff[SECT_CODE].index, fout->file);
}

/*
 * Returns sizeof(data) in bytes
 */
int sizeof_data(int data_type)
{
	if (data_type <= VOID)
	{
		switch (data_type)
		{
		case UINT8_T:
		case SINT8_T:
			return 1;
		case UINT16_T:
		case SINT16_T:
			return 2;
		case UINT32_T:
		case SINT32_T:
			return 4;
		case UINT64_T:
		case SINT64_T:
		case UINT8_PTR_T:
		case SINT8_PTR_T:
		case UINT16_PTR_T:
		case SINT16_PTR_T:
		case UINT32_PTR_T:
		case SINT32_PTR_T:
		case UINT64_PTR_T:
		case SINT64_PTR_T:
			return 8;
		}
	}
	/* The plan here is for all values above VOID to be indexes into a table of
	 * structs. This is because sizeof(struct) varies.
	 */
	else
	{
		assert(0);
	}
}
