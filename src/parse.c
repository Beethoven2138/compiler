#include <parse.h>
#include <lex.h>
#include <file.h>




static void MOVE(OPERAND dest, OPERAND src)
{
	if (dest.type == src.type && dest.value == src.value && dest.data_type == src.data_type)
		return;

	else if (dest.type == TREGISTER)
	{
		if (src.type == TIMMEDIATE)
			MOV_R64I(dest.value, src.value, sizeof_data(dest.data_type));
		else if (src.type == TREGISTER)
		{
			if ((!PTR(dest.data_type) && !PTR(src.data_type)) || (PTR(dest.data_type) && PTR(src.data_type)))
				MOV_R64R64(dest.value, src.value, sizeof_data(dest.data_type));
			else if (PTR(dest.data_type) && !PTR(src.data_type))//HMM.....
				MOV_R64derefR64(dest.value, src.value, 8);
			else
				MOV_R64R64deref(dest.value, src.value, 8);
			//MOV_R64R64(dest.value, src.value, sizeof_data(dest.data_type));
		}
		else if (src.type == TOFFSET)
		{
			if (!PTR(dest.data_type))
				MOV_R64OFF(dest.value, src.off, src.off_type, src.base_ptr, sizeof_data(dest.data_type), src.pos);
			else
				LEA(dest.value, src.off, src.off_type, src.base_ptr, sizeof_data(dest.data_type), src.pos);
		}
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			if ((!PTR(dest.data_type) && !PTR(src.data_type)) || (PTR(dest.data_type) && PTR(src.data_type)))
				MOV_R64D(dest.value, src.id, sizeof_data(src.data_type));
			else/* if (PTR(dest.data_type) && !PTR(src.data_type))*/
				MOV_R64ADR(dest.value, src.id, QWORD);
			/*else
			  assert(0);*/
		}
	}
	else if (dest.type == TOFFSET)
	{
		if (src.type == TIMMEDIATE)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64I(tmp, src.value, sizeof_data(dest.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type), dest.pos);
			reg_free(tmp);
		}
		else if (src.type == TREGISTER)
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, src.value, sizeof_data(dest.data_type), dest.pos);
		else if (src.type == TOFFSET)
		{
			/*x86 doesn't allow for MOV addr, addr
			  so src needs to be moved to a register first
			*/
			REGISTER tmp = reg_alloc();
			//TODO: make sure sizeof_data is also there in stack offsets
			MOV_R64OFF(tmp, src.off, src.off_type, src.base_ptr, sizeof_data(src.data_type), src.pos);
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type), dest.pos);
			reg_free(tmp);
		}
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64D(tmp, src.id, sizeof_data(src.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type), dest.pos);
			reg_free(tmp);
		}
	}
	else if (dest.type == TSEG_DATA || dest.type == TSEG_BSS)
	{
		if (src.type == TREGISTER)
		{
			MOV_DR64(dest.id, src.value, sizeof_data(src.data_type));
		}
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64D(tmp, src.id, sizeof_data(src.data_type));
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		else if (src.type == TOFFSET)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64OFF(tmp, src.off, src.off_type, src.base_ptr, QWORD, src.pos);
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		else if (src.type == TIMMEDIATE)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64I(tmp, src.value, sizeof_data(dest.data_type));
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
	}
}

static void MOVE_deref(OPERAND dest, OPERAND src)
{
	if (dest.type == src.type && dest.value == src.value && dest.data_type == src.data_type)
		return;

	else if (dest.type == TREGISTER)
	{
		if (src.type == TIMMEDIATE)
			MOV_R64I(dest.value, src.value, sizeof_data(dest.data_type));
		else if (src.type == TREGISTER)
		{
			if ((!PTR(dest.data_type) && !PTR(src.data_type)) || (PTR(dest.data_type) && PTR(src.data_type)))
				MOV_R64R64(dest.value, src.value, sizeof_data(dest.data_type));
			else if (PTR(dest.data_type) && !PTR(src.data_type))//HMM.....
				MOV_R64derefR64(dest.value, src.value, 8);
			else
				MOV_R64R64deref(dest.value, src.value, 8);
			//MOV_R64R64(dest.value, src.value, sizeof_data(dest.data_type));
		}
		else if (src.type == TOFFSET)
		{
			MOV_R64OFF(dest.value, src.off, src.off_type, src.base_ptr, sizeof_data(dest.data_type), src.pos);
		}
		/*
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			MOV_R64D(dest.value, src.id, sizeof_data(src.data_type));
			}*/
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			if ((!PTR(dest.data_type) && !PTR(src.data_type)) || (PTR(dest.data_type) && PTR(src.data_type)))
				MOV_R64D(dest.value, src.id, sizeof_data(src.data_type));
			else/* if (PTR(dest.data_type) && !PTR(src.data_type))*/
				MOV_R64ADR(dest.value, src.id, QWORD);
			/*else
			  assert(0);*/
		}
	}
	else if (dest.type == TOFFSET)
	{
		if (src.type == TIMMEDIATE)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64I(tmp, src.value, sizeof_data(dest.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type), dest.pos);
			reg_free(tmp);
		}
		else if (src.type == TREGISTER)
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, src.value, sizeof_data(dest.data_type), dest.pos);
		else if (src.type == TOFFSET)
		{
			/*x86 doesn't allow for MOV addr, addr
			  so src needs to be moved to a register first
			*/
			REGISTER tmp = reg_alloc();
			//TODO: make sure sizeof_data is also there in stack offsets
			MOV_R64OFF(tmp, src.off, src.off_type, src.base_ptr, sizeof_data(src.data_type), src.pos);
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type), dest.pos);
			reg_free(tmp);
		}
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64D(tmp, src.id, sizeof_data(src.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type), dest.pos);
			reg_free(tmp);
		}
	}
	else if (dest.type == TSEG_DATA || dest.type == TSEG_BSS)
	{
		if (src.type == TREGISTER)
		{
			MOV_DR64(dest.id, src.value, sizeof_data(src.data_type));
		}
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64D(tmp, src.id, sizeof_data(src.data_type));
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		else if (src.type == TOFFSET)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64OFF(tmp, src.off, src.off_type, src.base_ptr, QWORD, src.pos);
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		else if (src.type == TIMMEDIATE)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64I(tmp, src.value, sizeof_data(dest.data_type));
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
	}
}

/*
uint64_t array[10];

array[4] = [array + sizeof(uint64_t) * 4]
*/

static void parse_factor(OPERAND *dest, bool deref /*are we dereferencing a pointer?*/)
{
	static int routine = 0;

	if (token.class != '(' && token.value != '*')
	{
		OPERAND src;
		src.data_type = dest->data_type;
		if (token.class == TNUMBER)
		{
			src.type = TIMMEDIATE;
			src.value = token.value;
		}
		else if (token.class == TIDENTIFIER)
		{
			src = *find_var(current_scope, token.id);
			read_token();
			if (token.class == '[')
			{
				src.type = TOFFSET;
				src.base_ptr = src.id;
				read_token();
				if (token.class == TNUMBER)
				{
					printf("\n%d\n", sizeof_data(src.data_type));
					src.off = token.value * sizeof_data(src.data_type-8);
					src.off_type = TIMMEDIATE;
					read_token();
					if (token.class != ']')
					{
						unread_token();
						goto normal;
					}
				}
				else
normal:
				{
					OPERAND tmp;
					tmp.type = TREGISTER;
					tmp.data_type = UINT64_T;
					tmp.value = reg_alloc();
					parse_logic(&tmp, deref);
					src.off = tmp.value;
					src.off_type = TREGISTER;
				}
			}
			else
			{
				unread_token();
			}
		}
		else if (token.class == TKEYWORD && token.value == SIZEOF)
		{
			read_token();
			read_token();
			src.type = TIMMEDIATE;
			src.value = sizeof_data(token.value);
			read_token();
		}
		else if (token.class == TFUNCTION)
		{
			call_function(token.id);
			src.type = TREGISTER;
			src.value = RAX;
		}
		if (!deref)
			MOVE(*dest, src);
		else
			MOVE_deref(*dest, src);
		read_token();
	}

	else if (token.class == '(')
	{
		OPERAND operand;
		operand.type = TREGISTER;
		operand.value = reg_alloc();
		operand.data_type = dest->data_type;
		read_token();

		parse_logic(&operand, deref);
		if (!deref)
			MOVE(*dest, operand);
		else
			MOVE_deref(*dest, operand);
		read_token();
		reg_free(operand.value);
	}
}



static void parse_prefix(OPERAND *dest, bool deref)
{
	if (token.class == TOPERATOR && token.value == '*')
	{
		OPERAND src;
		//src.data_type = dest->data_type;//or maybe it should be dest->data_type + 8?
		//src.data_type = dest->data_type + 8;//we'll try...
		src.data_type = (dest->data_type < 8) ? dest->data_type + 8 : dest->data_type;
		src.value = reg_alloc();
		src.type = TREGISTER;
		read_token();
		parse_factor(&src, true);
		if (dest->type == TREGISTER)
			MOV_R64R64deref(dest->value, src.value, sizeof_data(dest->data_type));
		else  if (dest->type == TSEG_DATA || dest->type == TSEG_BSS)
		{
			OPERAND tmp;
			tmp.data_type = src.data_type;
			tmp.value = reg_alloc();
			MOV_R64D(tmp.value, registers[src.value], sizeof_data(src.data_type));
			reg_free(tmp.value);
			MOV_DR64(dest->id, tmp.value, sizeof_data(dest->data_type));
		}
		reg_free(src.value);
	}
	else if (token.class == TOPERATOR && token.value == '&')
	{
		if (!PTR(dest->data_type))
			assert(0);
		read_token();
		parse_factor(dest, deref);
	}

	else
	{
		parse_factor(dest, deref);
	}
}

static void parse_term(OPERAND *dest, bool deref)
{
	parse_prefix(dest, deref);

	while (token.class == TOPERATOR && (token.value == '*' || token.value == '/' || token.value == '%'))
	{
		if (token.value == '*')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_factor(&operand, deref);
			OPERAND tmp;
			tmp.type = TREGISTER;
			tmp.value = RAX;
			tmp.data_type = dest->data_type;
			MOVE(tmp, *dest);
			MUL_R64(operand.value, sizeof_data(dest->data_type));
			MOVE(*dest, tmp);
			reg_free(operand.value);
		}
		else if (token.value == '/')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_factor(&operand, deref);
			OPERAND tmp;
			tmp.type = TREGISTER;
			tmp.value = RAX;
			tmp.data_type = dest->data_type;
			MOVE(tmp, *dest);
			DIV_R64(operand.value, sizeof_data(operand.data_type));
			MOVE(*dest, tmp);
			reg_free(operand.value);
		}
		else if (token.value == '%')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_factor(&operand, deref);
			OPERAND tmp;
			tmp.type = TREGISTER;
			tmp.value = RAX;
			tmp.data_type = dest->data_type;
			MOVE(tmp, *dest);
			DIV_R64(operand.value, sizeof_data(operand.data_type));
			//The remainder of a division is stored in RDX
			tmp.value = RDX;
			MOVE(*dest, tmp);
			reg_free(operand.value);
		}
	}
}


/*
If immediate value, don't move it to a register, do addition immediately
If register, same
Only have to move immediate value to a register in MUL and DIV
*/

static void parse_expression(OPERAND *dest, bool deref)
{
	parse_term(dest, deref);

	static int routine = 0;

	while (token.class == TOPERATOR && (token.value == '+' || token.value == '-'))
	{
		if (token.value == '+')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_term(&operand, deref);
			if (!PTR(dest->data_type))
			{
				if (operand.type == TREGISTER && dest->type == TREGISTER)
					ADD_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
				else if (operand.type == TREGISTER && dest->type != TREGISTER)
				{
					OPERAND tmp; tmp.type = TREGISTER; tmp.value = reg_alloc(); tmp.data_type = dest->data_type;
					MOVE(tmp, *dest);
					ADD_R64R64(tmp.value, operand.value, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
					MOVE(*dest, tmp);
					reg_free(tmp.value);
				}
				else
					assert(0);
			}
			else
			{
				PUSH(RDX, 8);
				PUSH(RAX, 8);
				MOV_R64I(RAX, sizeof_data(dest->data_type - 8), 8);
				MUL_R64(operand.value, 8);
				if (operand.type == TREGISTER && dest->type == TREGISTER)
				{
					ADD_R64R64(dest->value, RAX, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
				}
				else if (operand.type == TREGISTER && dest->type != TREGISTER)
				{
					OPERAND tmp; tmp.type = TREGISTER; tmp.value = reg_alloc(); tmp.data_type = dest->data_type;
					MOVE(tmp, *dest);
					ADD_R64R64(tmp.value, RAX, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
					MOVE(*dest, tmp);
					reg_free(tmp.value);
				}
				else
					assert(0);
				POP(RAX, 8);
				POP(RDX, 8);
			}
			reg_free(operand.value);
		}
		else if (token.value == '-')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_term(&operand, deref);
			if (!PTR(dest->data_type))
			{
				if (operand.type == TREGISTER && dest->type == TREGISTER)
					SUB_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
				else if (operand.type == TREGISTER && dest->type != TREGISTER)
				{
					OPERAND tmp; tmp.type = TREGISTER; tmp.value = reg_alloc(); tmp.data_type = dest->data_type;
					MOVE(tmp, *dest);
					SUB_R64R64(tmp.value, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
					MOVE(*dest, tmp);
					reg_free(tmp.value);
				}
				else
					assert(0);
			}
			else
			{
				PUSH(RDX, 8);
				PUSH(RAX, 8);
				MOV_R64I(RAX, sizeof_data(dest->data_type - 8), 8);
				MUL_R64(operand.value, 8);
				if (operand.type == TREGISTER && dest->type == TREGISTER)
				{
					SUB_R64R64(dest->value, RAX, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
				}
				else if (operand.type == TREGISTER && dest->type != TREGISTER)
				{
					OPERAND tmp; tmp.type = TREGISTER; tmp.value = reg_alloc(); tmp.data_type = dest->data_type;
					MOVE(tmp, *dest);
					SUB_R64R64(tmp.value, RAX, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
					MOVE(*dest, tmp);
					reg_free(tmp.value);
				}
				else
					assert(0);
				POP(RAX, 8);
				POP(RDX, 8);
			}
			reg_free(operand.value);
		}
	}
}

static void parse_shift(OPERAND *dest, bool deref)
{
	parse_expression(dest, deref);
	while (token.class == TOPERATOR && (token.value == SLEFT || token.value == SRIGHT))
	{
		if (token.value == SLEFT)
		{
			read_token();
			assert(token.class == TNUMBER);
			SHL(dest->value, token.value, sizeof_data(dest->data_type));
		}
		else if (token.value == SRIGHT)
		{
			read_token();
			assert(token.class == TNUMBER);
			SHR(dest->value, token.value, sizeof_data(dest->data_type));
		}
		read_token();
	}
}

static void parse_relation(OPERAND *dest, bool deref)
{
	parse_shift(dest, deref);

	static int routine = 0;

	//read_token();

	while (token.class == TOPERATOR && (token.value >= EQUAL && token.value <= LESS_EQUAL))
	{
		if (token.value == EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand, deref);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JNE(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
		else if (token.value == NOT_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand, deref);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JE(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
		else if (token.value == GREATER)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand, deref);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JBE(else_routine);//TODO: add in JL for signed values;
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
		else if (token.value == LESS)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand, deref);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JAE(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
		else if (token.value == LESS_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand, deref);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JA(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
		else if (token.value == GREATER_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand, deref);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JB(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
	}
}


static void parse_logic(OPERAND *dest, bool deref)
{
	static int routine = 0;

	parse_relation(dest, deref);

	while (token.class == TOPERATOR && (token.value == AND || token.value == OR))
	{
		if (token.value == AND)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_relation(&operand, deref);
			AND_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rlog_%d", routine);
			sprintf(end_routine, "_rendlog_%d", routine);
			routine++;
			JE(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
		else if (token.value == OR)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_relation(&operand, deref);
			OR_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rlog_%d", routine);
			sprintf(end_routine, "_rendlog_%d", routine);
			routine++;
			JE(else_routine);
			MOV_R64I(dest->value, 1, sizeof_data(dest->data_type));
			JMP(end_routine);
			write_str(else_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			MOV_R64I(dest->value, 0, sizeof_data(dest->data_type));
			write_str(end_routine, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			reg_free(operand.value);
		}
	}
}

static void parse_assignment(OPERAND *dest, bool deref)
{
	unread_token();
	if (!(token.class == TOPERATOR && token.value == '*'))
	{
		read_token();
		//parse_factor(dest, false);
		parse_expression(dest, deref);
		if (token.class == ';' || token.class == '}')
			return;
		read_token();
		if (token.class != ';' && token.class != '}')
		{
			OPERAND tmp;
			tmp.type = TREGISTER;
			tmp.value = reg_alloc();
			tmp.data_type = dest->data_type;
			parse_expression(&tmp, deref);
			//parse_shift(&tmp);
			MOVE(*dest, tmp);
		}
	}
	else
	{
		//OPERAND tmp = {.type = TREGISTER, .value = reg_alloc(), .data_type = dest->data_type-8};don't think that's right...
		OPERAND tmp = {.type = TREGISTER, .value = reg_alloc(), .data_type = dest->data_type - 8};
		OPERAND tmp_dest = {.type = TREGISTER, .value = reg_alloc(), .data_type = dest->data_type};
		parse_expression(&tmp_dest, deref);
		read_token();
		parse_expression(&tmp, deref);
		MOV_R64derefR64(tmp_dest.value, tmp.value, sizeof_data(tmp.data_type));
		reg_free(tmp_dest.value);
		reg_free(tmp.value);
	}
}


static void parse_declaration(int flags)
{
	int data_type = token.value;


	read_token();

	if (token.class == TIDENTIFIER)
	{
		OPERAND var;
		var.data_type = data_type;
		var.id = token.id;

		if (current_scope != parent && (flags & 0x100) == 0)
		{
			bool skip = false;
			/* Only for when there's the register keyword*/
			if (flags & 64)
			{
				var.type = TREGISTER;
				var.value = reg_alloc();
			}
			else
			{
				read_token();
				if (token.class == '[')
				{
					var.data_type += 8;//since it's now a pointer;
					var.base_ptr = spec_registers[RBP];
					var.type = TOFFSET;
					var.off = current_scope->offset;
					var.off_type = TIMMEDIATE;
					var.pos = false;
					OPERAND arr_size = {.type = TREGISTER, .data_type = UINT64_T, .value = reg_alloc()};
					read_token();
					parse_expression(&arr_size, false);
					read_token();
					PUSH(RAX, 8);
					MOV_R64I(RAX, sizeof_data(var.data_type-8), sizeof_data(UINT64_T));
					MUL_R64(arr_size.value, sizeof_data(UINT64_T));
					SUB_RSPR64(RAX);
					POP(RAX, 8);
					reg_free(arr_size.value);
					skip = true;
				}
				else
				{
					unread_token();
					var.base_ptr = spec_registers[RBP];
					var.type = TOFFSET;
					var.off = current_scope->offset;
					var.off_type = TIMMEDIATE;
					var.pos = false;
				}
			}
			add_variable(var, current_scope);
			if (!skip)
			{
				read_token();
				if (token.class != ';')
				{
					unread_token();
					parse_assignment(&var, false);
				}
			}
		}
		else
		{
			read_token();
			/*
			 * Here, the initializer element has to be constant.
			 * I can't handle complex operations like 5 - (34 /37) etc
			 * Since all the functions that do that output registers.
			 * I'll just have to hope that the preprocessor substitutes the
			 * result of the operation. If not, I'm in trouble :D.
			 */
			if (token.value == '=')
			{
				var.type = TSEG_DATA;
				read_token();
				assert(token.class == TNUMBER);
				var.value = token.value;
				//TODO: add arrays and structs
				add_data(var.id, var.data_type, var.value, flags & 0x1);
				read_token();
			}
			else
			{
				var.type = TSEG_BSS;
				/* For the time being assume that
				 * size in add_bss is always 1.
				 * Won't be true with arrays though.
				 */
				int size = 1;
				/*array*/
				if (token.class == '[')
				{
					read_token();
					size = token.value;
					read_token();
					add_bss(var.id, var.data_type, size, flags & 0x1);
					//var.data_type += 8;
				}
				else
					add_bss(var.id, var.data_type, size, flags & 0x1);
				read_token();
			}
			add_variable(var, current_scope);
		}
	}
	else if (token.class == TFUNCTION)
	{
		/*if (current_scope != parent)
		  assert(0);*/
		current_scope = parent;
		OPERAND *vars;
		FUNCTION *func;
		func = find_function(token.id);
		if (func != NULL)
		{
			while (token.class != '{')
				read_token();
			goto scope;
		}
		else
		/*This means that the function hasn't already been declared
		  which means that the arguments of the function have to be sorted out.
		  If there already was a declaration, this step is skipped*/
		{
			vars = (OPERAND*)malloc(sizeof(OPERAND));
			func = create_function(token.id, vars, 0, sizeof_data(data_type), 0);

			func->type = data_type;

			read_token();

			if (token.value != '(')
				assert(0);
			read_token();
			OPERAND var;
			int offset = 16;

			while (token.class != ')')
			{
				if (token.class == TKEYWORD)
				{
					var.data_type = token.value;//CHECK TO SEE IF THEY USE THE SAME ENUM!
				}
				else if (token.class == TIDENTIFIER)
				{
					var.pos = true;
					var.id = token.id;
					var.type = TOFFSET;
					var.off = offset;
					var.off_type = TIMMEDIATE;
					var.base_ptr = spec_registers[RBP];
					add_funcvar(func, var);
					var.type = 0;
					var.id = 0;
					switch (sizeof_data(var.data_type))
					{
					case QWORD:
						offset += 8;
						break;
					case DWORD:
						offset += 4;
						break;
					case WORD:
						offset += 2;
						break;
					case BYTE:
						++offset;
						break;
					}
				}
				read_token();
			}
			add_function(func);
			read_token();
scope:
			if (token.class == '{')
			{
				if (flags & 0x1)
				{
					write_strn("global ", 6, SECT_CODE);
				}
				write_str(func->name, SECT_CODE);
				write_strn(":\n", 2, SECT_CODE);

				current_scope = add_scope(parent, NULL, NULL, NULL, func);
				func_prolog();
			        REGISTER reg;

				for (int i = 0; i < func->var_count; i++)
					add_variable(func->vars[i], current_scope);

				current_scope->offset = 16;
				/*TODO: parse_scope sucks; make parse_statement except
				 it has while (token.class != '}' instead of TEOF)*/
				int local_vars_sum = 0;
				int left_cnt = 1;
				int right_cnt = 0;
				int index = fin->buff->index;
				TOKEN old_prev = prev_token;
				TOKEN old = token;
				int old_token_list_index = token_list_index;
				while (right_cnt < left_cnt)
				{
					read_token();
					if (token.class == '{')
						++left_cnt;
					else if (token.class == '}')
						++right_cnt;
					else if (token.class == TKEYWORD && token.value < STRUCT)
						local_vars_sum += sizeof_data(token.value);
				}
				fin->buff->index = index;
				prev_token = old_prev;
				token = old;
				token_list_index = old_token_list_index;
				current_scope->local_var_sum = local_vars_sum;
				writec(9, SECT_CODE);
				write_strn("SUB RSP, ", 9, SECT_CODE);
				char tmp1[100];
				sprintf(tmp1, "%d\n", local_vars_sum);
				write_str(tmp1, SECT_CODE);
				parse_statement('}');
				for (int i = 0; i < func->var_count; i++)
				{
					reg_free(func->vars[i].value);
				}
				if (func->type == VOID && strcmp(func->name, "main"))
					func_epilog();
				else if (!strcmp(func->name, "main"))
					EXIT();
				free_scope(current_scope);
				current_scope = parent;
			}
		}
		read_token();
	}
}

static void parse_condition(void)
{
	static int routine = 0;

	OPERAND dest;
	dest.type = TREGISTER;
	dest.value = reg_alloc();
	dest.data_type = UINT64_T;

	read_token();
	read_token();
	parse_logic(&dest, false);
	//parse_expression(&dest);

	char else_routine[100];
	char end_routine[100];
	sprintf(else_routine, "rcondelse_%d", routine);
	sprintf(end_routine, "rcondend_%d", routine);


	routine++;

	OPERAND tmp;
	tmp.type = TIMMEDIATE;
	tmp.value = 1;
	tmp.data_type = UINT64_T;

	CMP(dest.value, tmp, sizeof_data(dest.data_type));

	reg_free(dest.value);

	JNE(else_routine);

	read_token();
	if (token.class == '{')
	{
		unsigned int offset = current_scope->offset;
		current_scope = add_scope(current_scope, NULL, NULL, NULL, NULL);
		current_scope->offset = offset;
		parse_statement('}');
		JMP(end_routine);
		read_token();//Check this
		current_scope = current_scope->parent;
		free_scope(current_scope->child);
	}
	else
	{
		printf("%d", token.class);
		assert(0);
	}

	write_str(else_routine, SECT_CODE);
	write_strn(":\n", 2, SECT_CODE);

	if (token.class == TKEYWORD && token.value == ELSE)
	{
		read_token();
		if (token.class == '{')
		{
			
			////DO THIS FOR ALL SUCH LOCATIONS!!!




			unsigned int offset = current_scope->offset;
			current_scope = add_scope(current_scope, NULL, NULL, NULL, NULL);
			current_scope->offset = offset;
			parse_statement('}');
			read_token();
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
		}
		else
			assert(0);
	}

	write_str(end_routine, SECT_CODE);
	write_strn(":\n", 2, SECT_CODE);

	reg_free(dest.value);

	/*if (cmp_op.class == TOPERATOR)
	{
		char else_routine[100];
		char end_routine[100];
		sprintf(else_routine, "rif_%d:", routine);
		sprintf(end_routine, "rend_%d:", routine);
		routine++;
		CMP(op1.value, op2);
		if (cmp_op.value == EQUAL)
		{
			reg_free(op1.value);
			reg_free(op2.value);
			JNE(else_routine);
			read_token();
			read_token();
			if (token.class == '{')
			{
				current_scope = add_scope(current_scope, NULL, NULL, NULL);
				parse_scope();
				JMP(end_routine);
				read_token();
				current_scope = current_scope->parent;
				free_scope(current_scope->child);
				//Currently all this stuff only works when there's {. Not for single statement subroutines
				if (token.class == TKEYWORD && token.value == ELSE)
				{
					write_str(else_routine);
					read_token();
					if (token.class == '{')
					{
						current_scope = add_scope(current_scope, NULL, NULL, NULL);
						parse_scope();
						read_token();
						current_scope = current_scope->parent;
						free_scope(current_scope->child);
					}
					else
						assert(0);
				}
				write_str(end_routine);
			}
			else
				assert(0);
		}
		else
			assert(0);
	}*/
}

static void parse_loop(void)
{
	static int routine = 0;
	if (token.value == WHILE)
	{
		char loop[100];
		char loop_end[100];
		sprintf(loop, "_loop%d", routine);
		sprintf(loop_end, "_loop_end%d", routine++);
		write_str(loop, SECT_CODE);
		write_strn(":\n", 2, SECT_CODE);
		unsigned int offset = current_scope->offset;
		current_scope = add_scope(current_scope, NULL, NULL, NULL, NULL);
		current_scope->offset = offset;
		read_token();
		assert(token.class == '(');
		OPERAND dest;
		dest.type = TREGISTER;
		dest.value = reg_alloc();

		read_token();
		//LOOK AT THIS AGAIN
		dest.data_type = UINT64_T;
		parse_logic(&dest, false);

		OPERAND tmp;
		tmp.type = TIMMEDIATE;
		tmp.value = 1;
		tmp.data_type = dest.data_type;

		CMP(dest.value, tmp, sizeof_data(dest.data_type));
		JNE(loop_end);
		while (token.class != '{')
			read_token();
		if (token.class == '{')
		{
			//EXPERIMENTAL!!!
			unsigned int offset = current_scope->offset;
			current_scope = add_scope(current_scope, NULL, NULL, NULL, NULL);
			current_scope->offset = offset;


			parse_statement('}');
			JMP(loop);
			write_str(loop_end, SECT_CODE);
			write_strn(":\n", 2, SECT_CODE);
			read_token();
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
		}
		else
		{
			printf("%d", token.class);
			assert(0);
		}
	}
	else if (token.value == FOR)
	{
		unsigned int offset = current_scope->offset;
		current_scope = add_scope(current_scope, NULL, NULL, NULL, NULL);
		current_scope->offset = offset;
		read_token();
		parse_statement(';');
		read_token();

		char loop[100];
		char loop_end[100];
		sprintf(loop, "_loop%d", routine);
		sprintf(loop_end, "_loop_end%d", routine++);
		write_str(loop, SECT_CODE);
		write_strn(":\n", 2, SECT_CODE);

		OPERAND dest;
		dest.type = TREGISTER;
		dest.value = reg_alloc();
		dest.data_type = UINT64_T;

		parse_logic(&dest, false);

		OPERAND tmp;
		tmp.type = TIMMEDIATE;
		tmp.value = 1;
		tmp.data_type = dest.data_type;

		CMP(dest.value, tmp, sizeof_data(dest.data_type));
		JNE(loop_end);


		/*
		The final part of the for loop happens at the end of the scope
		therefore the code for it has to be added at the end
		so file_pos is there so that the program can go back to it
		at the end of the scope
		*/
		int file_pos = fin->buff->index;

		read_token();

		while (token.class != '{')
			read_token();

		parse_statement('}');
		int end_loop_pos = fin->buff->index;
		fin->buff->index = file_pos;
		parse_statement(')');
		JMP(loop);
		write_str(loop_end, SECT_CODE);
		write_strn(":\n", 2, SECT_CODE);
		fin->buff->index = end_loop_pos;
		read_token();
		current_scope = current_scope->parent;
		free_scope(current_scope->child);
	}
}


void parse_statement(int stop)
{
	static int scope = 0;
	char scope_start[100];
	char scope_end[100];
	sprintf(scope_start, "_scope_start%d", scope);
	sprintf(scope_end, "_scope_end%d", scope++);
	write_str(scope_start, SECT_CODE);
	write_strn(":\n", 2, SECT_CODE);

	int type;
	read_token();
	do
	{
		if (token.class == TKEYWORD && ((token.value >= UINT64_T && token.value <= UINT8_PTR_T) || token.value == VOID))
			parse_declaration(0);
		else if (token.class == TKEYWORD && token.value == K_REGISTER)
		{
			assert(current_scope != parent);
			read_token();
			assert(token.class == TKEYWORD);
			parse_declaration(64);
		}
		else if (token.class == TKEYWORD && token.value == UNION)
		{
		}
		else if (token.class == TKEYWORD && token.value == EXTERN)
		{
			assert(current_scope == parent);
			write_strn("extern ", 6, SECT_CODE);
			read_token();
			assert(token.class == TKEYWORD);
			parse_declaration(EXTERN_LINK);
		}
		else if (token.class == TKEYWORD && token.value == STATIC)
		{
			read_token();
			assert(token.class == TKEYWORD);
			parse_declaration(0x100);
		}
		else if (token.class == TKEYWORD && token.value == STRUCT)
		{
			read_token();
			assert(token.class == TIDENTIFIER);
			parse_declaration(0x1000);
		}
		else if (token.class == TIDENTIFIER)
		{
			OPERAND *var = find_var(current_scope, token.id);
			assert(var);
			parse_assignment(var, false);
		}
		else if (token.class == TKEYWORD && token.value == IF)
		{
			parse_condition();
		}
		else if (token.class == TKEYWORD && (token.value == WHILE ||
						     token.value == FOR))
		{
			parse_loop();
		}
		else if (token.class == TKEYWORD && token.value == RETURN)
		{
			read_token();
			if (token.class != ';')
			{
				OPERAND rax = {.type = TREGISTER, .value = RAX, .data_type = current_scope->func->type};
			        OPERAND tmp = {.type = TREGISTER, .data_type = current_scope->func->type, .value = reg_alloc()};
				parse_expression(&tmp, false);
				MOVE(rax, tmp);
				reg_free(tmp.value);
				char tmp1[100];
				sprintf(tmp1, "%d\n", current_scope->local_var_sum);
				writec(9, SECT_CODE);
				write_strn("ADD RSP, ", 9, SECT_CODE);
				write_str(tmp1, SECT_CODE);
				writec(9, SECT_CODE);
				func_epilog();
			}
			else
			{
				char tmp1[100];
				sprintf(tmp1, "%d\n", current_scope->local_var_sum);
				writec(9, SECT_CODE);
				write_strn("ADD RSP, ", 9, SECT_CODE);
				write_str(tmp1, SECT_CODE);
				writec(9, SECT_CODE);
				func_epilog();
			}
		}
		else if (token.class == TFUNCTION)
		{
			call_function(token.id);

			read_token();
		}
		else if (token.class == '{')
		{
			current_scope = add_scope(current_scope, NULL, NULL, NULL, NULL);
			parse_statement('}');
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
			read_token();
		}
		else if (token.class == TKEYWORD && token.value == BREAK)
		{
			JMP(scope_end);
			read_token();
		}
		else if (token.class == TKEYWORD && token.value == CONTINUE)
		{
			JMP(scope_start);
			read_token();
		}
		else if (token.class == ';')
			read_token();
		else if (token.class == TOPERATOR && token.value == '*')
		{
			read_token();
			OPERAND tmp_ptr = {.data_type = UINT64_PTR_T, .type = TREGISTER, .value = reg_alloc()};
			parse_factor(&tmp_ptr, true);
			//OPERAND r_tmp = {.data_type = tmp_ptr.data_type, .type = TREGISTER, .value = reg_alloc()};
			OPERAND r_tmp = {.data_type = (tmp_ptr.data_type < 8) ? tmp_ptr.data_type : tmp_ptr.data_type - 8,
				         .type = TREGISTER, .value = reg_alloc()};
			read_token();
			if (token.class == TOPERATOR && token.value == '*')
			{
				//read_token();
				r_tmp.data_type += 8;
				parse_assignment(&r_tmp, true);
				if (r_tmp.data_type < 8)
					MOV_R64R64deref(r_tmp.value, r_tmp.value, 8);
			}
			else
				parse_assignment(&r_tmp, true);//CHECK THIS	
			MOV_R64derefR64(tmp_ptr.value, r_tmp.value, sizeof_data(tmp_ptr.data_type));
			reg_free(tmp_ptr.value);
			reg_free(r_tmp.value);
		}
	} while (token.class != stop && token.class != TEOF);
	write_str(scope_end, SECT_CODE);
	write_strn(":\n", 2, SECT_CODE);
}

static void add_variable(OPERAND var, SCOPE *scope)
{
	if (scope->var_length == 0)
	{
		scope->vars = (OPERAND*)malloc(sizeof(OPERAND));
		scope->var_length++;
	}
	else if (scope->var_index == scope->var_length)
	{
		scope->vars = (OPERAND*)realloc(scope->vars, sizeof(OPERAND) * (scope->var_length+1));
		scope->var_length++;
	}
	scope->vars[scope->var_index++] = var;
	//ENSURE THAT VAR.DATA_TYPE IS SET BEFORE HERE!!!
	scope->offset += sizeof_data(var.data_type);
}

static SCOPE* add_scope(SCOPE *parent, SCOPE *child, SCOPE *prev, SCOPE *next, FUNCTION *func)
{
	SCOPE *new = (SCOPE*)malloc(sizeof(SCOPE));
	new->parent = parent;
	new->child = child;
	new->prev = prev;
	new->next = next;

	if (parent)
	{
		new->parent->child = new;
		new->func = parent->func;
	}

	new->var_length = 0;
	new->var_index = 0;
	new->vars = NULL;
	//CHECK THIS!!!
	new->offset = 16;
	if (func != NULL)
		new->func = func;
	return new;
}

static void free_scope(SCOPE *scope)
{
	if (scope == NULL)
		return;

	for (int i = 0; i < scope->var_length && i < scope->var_index; i++)
	{
		if (scope->vars[i].type == TREGISTER)
			reg_free(scope->vars[i].value);
		free(scope->vars[i].id);
	}
	if (scope->next != NULL || scope->prev != NULL)
	{
		if (scope->prev != NULL)
		{
			scope->prev->next = scope->next;
			if (scope->next != NULL)
				scope->next->prev = scope->prev;
		}

		else if (scope->next != NULL)
			scope->next->prev = scope->prev;
	}
	else
	{
		if (scope->child != NULL)
		{
			scope->child->parent = scope->parent;
			if (scope->parent != NULL)
				scope->parent->child = scope->child;
		}
		else if (scope->parent != NULL)
			scope->parent->child = scope->child;
	}
	free(scope);
}

void free_whole_scope(SCOPE *parent)
{
	while (parent != NULL && parent->child != NULL)
	{
		SCOPE *child = parent->child;
		while (1)
		{
			child = child->next;
			if (child && child->prev)
				free_scope(child->prev);
			else
				break;
		}
	}
	if (parent)
		free_scope(parent);
}

void init_scope(void)
{
	parent = add_scope(NULL, NULL, NULL, NULL, NULL);
	current_scope = parent;
}

static OPERAND* find_var(SCOPE *scope, char *id)
{
	SCOPE *tmp = scope;
	OPERAND *op;
	for (tmp; tmp != NULL; tmp = tmp->parent)
	{
		op = find_var_in_scope(*tmp, id);
		if (op)
			return op;
	}
	assert(0);
}

static OPERAND* find_var_in_scope(SCOPE scope, char *id)
{
	for (int i = 0; i < scope.var_length; i++)
	{
		if (scope.vars[i].id != NULL && (!strcmp(id, scope.vars[i].id)))
			return &(scope.vars[i]);
	}
	return 0;
}

static void add_function(FUNCTION *func)
{
	if (func_list.index == func_list.length)
		func_list.funcs = realloc(func_list.funcs, (func_list.length + 1)*sizeof(FUNCTION*));
	func_list.length++;

	func_list.funcs[func_list.index++] = func;
}

static FUNCTION* create_function(char *name, OPERAND *vars, int var_count, int type, unsigned int offset)
{
	FUNCTION *function = (FUNCTION*)malloc(sizeof(FUNCTION));
	function->name = name;
	function->vars = vars;
	function->type = type;
	function->var_count = var_count;
	function->offset = offset;
	return function;
}

static FUNCTION* find_function(const char *name)
{
	for (int i = 0; i < func_list.length; i++)
	{
		if (!strcmp(name, func_list.funcs[i]->name))
			return func_list.funcs[i];
	}
	return NULL;
}

void free_functions(void)
{
	for (int i = 0; i < func_list.length; i++)
	{
		free(func_list.funcs[i]->name);
		free(func_list.funcs[i]->vars);
	}
	free(func_list.funcs);
}

static void init_functions(void)
{
	func_list.funcs = 0;
	func_list.length = 0;
	func_list.index = 0;
}

static void add_funcvar(FUNCTION *func, OPERAND var)
{
	if (func->var_count > 0)
		func->vars = (OPERAND*)realloc(func->vars, sizeof(OPERAND)*(func->var_count + 1));
	func->vars[func->var_count] = var;
	func->var_count++;
	//ensure that the data type is set before calling this function
	func->offset += sizeof_data(var.data_type);
}

static void call_function(const char *func_name)
{
	read_token();
        FUNCTION *func = find_function(func_name);
	if (func->var_count == 0)
	{
		do {read_token();} while (token.value != ')');
		read_token();
		CALL(func_name);
		return;
	}
        OPERAND *vars = (OPERAND*)malloc(func->var_count * sizeof(OPERAND));
	int sum = 0;
	//The position in the file of the argument we need to push onto the stack.
	int *arg_pos_stack = (int*)malloc(func->var_count * sizeof(int));
	TOKEN *arg_token_stack = (TOKEN*)malloc(func->var_count * sizeof(TOKEN));
	int *token_index = (int*)malloc(func->var_count * sizeof(int)); //The indexes in the file corresponding to the above tokens
	int arg_index = 1;
	int end_pos = 0; //position of the final ')' of the function
	arg_pos_stack[0] = fin->buff->index;
	arg_token_stack[0] = token;
	token_index[0] = fin->buff->index;
	TOKEN end_token;
	TOKEN end_prev_token;
	int end_token_list_index;
	while (arg_index < func->var_count)
	{
		read_token();
		/*In case a function is called within a function argument.
		  For example, a situation like func1(a,b,func2(c,d),e);*/
		if (token.class == '(')
		{
			/*We follow a similar procedure to what we did in parse_declaration,
			  counting the number of ( and ), and waiting for when they're equal*/
			int left_cnt = 1;
			int right_cnt = 0;
			while (right_cnt < left_cnt)
			{
				read_token();
				if (token.class == '(')
					++left_cnt;
				else if (token.class == ')')
					++right_cnt;
			}
		}
		if (token.class == ',')
		{
			arg_pos_stack[arg_index] = fin->buff->index;
			read_token();
			arg_token_stack[arg_index] = token;
			token_index[arg_index++] = fin->buff->index;
			unread_token();
		}
	}

	//We now work backwards, pushing the arguments onto the stack
	while (--arg_index >= 0)
	{
		OPERAND tmp = {.type = TREGISTER, .data_type = func->vars->data_type, .value = reg_alloc()};
		sum += sizeof_data(tmp.data_type);
		fin->buff->index = arg_pos_stack[arg_index];
		token = arg_token_stack[arg_index];
		fin->buff->index = token_index[arg_index];
		parse_expression(&tmp, false);
		if (arg_index == func->var_count - 1)
		{
			end_pos = fin->buff->index;
			end_token = token;
			end_token_list_index = token_list_index;
			end_prev_token = prev_token;
		}
		PUSH(tmp.value, sizeof_data(tmp.data_type));
		reg_free(tmp.value);
	}
	if (end_pos != 0)
		fin->buff->index = end_pos;
	prev_token = end_prev_token;
	token = end_token;
	token_list_index = end_token_list_index;
	CALL(func_name);
	writec(9, SECT_CODE);
	write_strn("ADD RSP, ", 9, SECT_CODE);
	char tmp[100];
	sprintf(tmp, "%d\n", sum);
	write_str(tmp, SECT_CODE);

	free(vars);
	free(arg_pos_stack);
	free(arg_token_stack);
	free(token_index);
}
