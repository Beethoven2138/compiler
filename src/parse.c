#include <parse.h>
#include <lex.h>
#include <file.h>




static void MOVE(OPERAND dest, OPERAND src)
{
	if (dest.type == src.type && dest.value == src.value)
		return;

	else if (dest.type == TREGISTER)
	{
		if (src.type == TIMMEDIATE)
			MOV_R64I(dest.value, src.value, sizeof_data(dest.data_type));
		else if (src.type == TREGISTER)
			MOV_R64R64(dest.value, src.value, sizeof_data(dest.data_type));
		else if (src.type == TOFFSET)
			if (!PTR(dest.data_type))
				MOV_R64OFF(dest.value, src.off, src.off_type, src.base_ptr, sizeof_data(dest.data_type));
		else
			LEA(dest.value, src.off, src.off_type, src.base_ptr, sizeof_data(dest.data_type));
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			if (!PTR(dest.data_type))
				MOV_R64D(dest.value, src.id, sizeof_data(src.data_type));
			else
				MOV_R64ADR(dest.value, src.id, QWORD);
		}
	}
	else if (dest.type == TOFFSET)
	{
		if (src.type == TIMMEDIATE)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64I(tmp, src.value, sizeof_data(dest.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		//MOV_OFFI(dest.off, dest.off_type, dest.base_ptr, src.value, sizeof_data(dest.data_type));
		else if (src.type == TREGISTER)
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, src.value, sizeof_data(dest.data_type));
		else if (src.type == TOFFSET)
		{
			/*x86 doesn't allow for MOV addr, addr
			  so src needs to be moved to a register first
			*/
			REGISTER tmp = reg_alloc();
			//TODO: make sure sizeof_data is also there in stack offsets
			MOV_R64OFF(tmp, src.off, src.off_type, src.base_ptr, sizeof_data(src.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		else if (src.type == TSEG_DATA || src.type == TSEG_BSS)
		{
			REGISTER tmp = reg_alloc();
			MOV_R64D(tmp, src.id, sizeof_data(src.data_type));
			MOV_OFFR64(dest.off, dest.off_type, dest.base_ptr, tmp, sizeof_data(dest.data_type));
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
			MOV_R64OFF(tmp, src.off, src.off_type, src.base_ptr, QWORD);
			MOV_DR64(dest.id, tmp, sizeof_data(dest.data_type));
			reg_free(tmp);
		}
		else if (src.type == TIMMEDIATE)
		{
			//MOV_DI(dest.id, src.value, sizeof_data(dest.data_type));//CHANGE THIS!! IMPORTANT CHANGED IT
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

static void parse_factor(OPERAND *dest)
{
	static int routine = 0;

	if (token.class != '(' && token.value != '*')
	{
		OPERAND src;
		src.data_type = dest->data_type;
		if (token.class == TNUMBER)
		{
			/*if (dest->type = TIMMEDIATE)
			{
				dest->value = token.value;
				read_token();
				return;
			}*/
			src.type = TIMMEDIATE;
			  src.value = token.value;
			//dest->type = TIMMEDIATE;/*experimental*/
			//dest->value = token.value;
			//return;
		}
		else if (token.class == TIDENTIFIER)
		{
			src = *find_var(current_scope, token.id);
			read_token();
			if (token.class == '[')
			{
				//dest->data_type = src.data_type;
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
					parse_logic(&tmp);
					src.off = tmp.value;
					src.off_type = TREGISTER;
				}
			}
			else
				unread_token();
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

		MOVE(*dest, src);
		read_token();
	}

	else if (token.class == '(')
	{
		OPERAND operand;
		operand.type = TREGISTER;
		operand.value = reg_alloc();
		operand.data_type = dest->data_type;
		read_token();

		parse_logic(&operand);
		//parse_expression(&operand);
		//MOV_R64R64(dest->value, operand.value, sizeof_data(dest->data_type));
		MOVE(*dest, operand);
		read_token();
		reg_free(operand.value);
	}
}



static void parse_prefix(OPERAND *dest)
{
	//unread_token();
	if (token.class == TOPERATOR && token.value == '*')
	{
		/*if (!(dest->data_type >= UINT64_PTR_T && dest->data_type <= UINT8_PTR_T))
			assert(0);*/
		OPERAND src;
		src.data_type = /*(PTR(dest->data_type)) ? dest->data_type - 8 : */dest->data_type;
		src.type = TOFFSET;
		src.off_type = 0;
		src.base_ptr = registers[reg_alloc()];
		src.off = 0;
		read_token();
		parse_factor(&src);
		if (dest->type == TREGISTER)
			//MOV_R64D(dest->value, registers[src.value], sizeof_data(dest->data_type));
			MOV_R64OFF(dest->value, src.off, src.off_type, src.base_ptr, sizeof_data(dest->data_type));
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
		//read_token();
	}
	else if (token.class == TOPERATOR && token.value == '&')
	{
		if (!PTR(dest->data_type))
			assert(0);
		read_token();
		parse_factor(dest);
	}

	else
	{
		//read_token();
		parse_factor(dest);
	}
}

static void parse_term(OPERAND *dest)
{
	//parse_factor(dest);
	parse_prefix(dest);

	while (token.class == TOPERATOR && (token.value == '*' || token.value == '/' || token.value == '%'))
	{
		if (token.value == '*')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_factor(&operand);
			MOV_R64R64(RAX, dest->value, sizeof_data(dest->data_type));
			MUL_R64(operand.value, sizeof_data(dest->data_type));
			MOV_R64R64(dest->value, RAX, sizeof_data(dest->data_type));
			reg_free(operand.value);
		}
		else if (token.value == '/')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_factor(&operand);
			MOV_R64R64(RAX, dest->value, sizeof_data(dest->data_type));
			DIV_R64(operand.value, sizeof_data(operand.data_type));
			MOV_R64R64(dest->value, RAX, sizeof_data(dest->data_type));
			reg_free(operand.value);
		}
		else if (token.value == '%')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_factor(&operand);
			MOV_R64R64(RAX, dest->value, sizeof_data(dest->data_type));
			DIV_R64(operand.value, sizeof_data(operand.data_type));
			//The remainder of a division is stored in RDX
			MOV_R64R64(dest->value, RDX, sizeof_data(dest->data_type));
			reg_free(operand.value);
		}
	}
}


/*
If immediate value, don't move it to a register, do addition immediately
If register, same
Only have to move immediate value to a register in MUL and DIV
*/

static void parse_expression(OPERAND *dest)
{
	parse_term(dest);

	static int routine = 0;

	while (token.class == TOPERATOR && (token.value == '+' || token.value == '-'))
	{
		if (token.value == '+')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			//const int tmp = operand.value;
			operand.data_type = dest->data_type;
			parse_term(&operand);
			if (operand.type == TREGISTER)
				ADD_R64R64(dest->value, operand.value, sizeof_data(dest->data_type),
					   sizeof_data(operand.data_type));
			/*else if (operand.type == TIMMEDIATE)
			{
				ADD_R64I(dest->value, operand.value, sizeof_data(dest->data_type));
			}
			else
				assert(0);
			reg_free(tmp);*/
			reg_free(operand.value);
		}
		else if (token.value == '-')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_term(&operand);
			SUB_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
			reg_free(operand.value);
		}
	}
}

static void parse_shift(OPERAND *dest)
{
	parse_expression(dest);
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

static void parse_relation(OPERAND *dest)
{
	parse_shift(dest);

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
			parse_shift(&operand);
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
			parse_shift(&operand);
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
			parse_shift(&operand);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JB(else_routine);//TODO: add in JL for signed values;
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
			parse_shift(&operand);
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
		else if (token.value == LESS_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JA(else_routine);
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
		else if (token.value == GREATER_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_shift(&operand);
			CMP(dest->value, operand, sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JB(else_routine);
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
	}
}


static void parse_logic(OPERAND *dest)
{
	static int routine = 0;

	parse_relation(dest);

	//read_token();

	while (token.class == TOPERATOR && (token.value == AND || token.value == OR))
	{
		if (token.value == AND)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_relation(&operand);
			AND_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rlog_%d", routine);
			sprintf(end_routine, "_rendlog_%d", routine);
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
		else if (token.value == OR)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			operand.data_type = dest->data_type;
			parse_relation(&operand);
			OR_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rlog_%d", routine);
			sprintf(end_routine, "_rendlog_%d", routine);
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
	}
}

static void parse_assignment(OPERAND *dest)
{
	/*if (dest->data_type >= UINT64_PTR_T && dest->data_type <= UINT8_PTR_T)
	{
		unread_token();
		if (token.class == TOPERATOR && token.value == '*')
		{
			OPERAND src;
			src.data_type = dest->data_type;
			src.type = TREGISTER;
			src.value = reg_alloc();
			read_token();
			parse_factor(&src);
			dest->type = TREGISTER;
			dest->data_type -= 8;
			dest->value = reg_alloc();
			MOV_R64D(dest->value, registers[src.value], sizeof_data(dest->data_type));
			//read_token();
			reg_free(src.value);
		}
		else
			read_token();
	}
	else
		read_token();*/
	unread_token();
	if (!(token.class == TOPERATOR && token.value == '*'))
	{
		read_token();
		parse_factor(dest);
		read_token();
		parse_shift(dest);
	}
	else
	{
		read_token();
		/*OPERAND tmp;
		tmp.type = TOFFSET;
		tmp.data_type = dest->data_type - 8;
		tmp.off = reg_alloc();
		tmp.base_ptr = "0";
		//MOVE(tmp, *dest);
		//parse_prefix(&tmp);
		parse_expression(&tmp);
		dest->data_type -= 8;
		MOVE(*dest, tmp);*/
		dest->data_type -= 8;
		parse_expression(dest);
		dest->type = TOFFSET;
		dest->off = dest->value;
		dest->off_type = TREGISTER;
		dest->base_ptr = "0";
		//parse_expression(dest);
		//dest->data_type -= 8;
		read_token();
		parse_expression(dest);
		reg_free(dest->off);
	}
	/*if (token.class == TOPERATOR && token.value == '=')
	{
		read_token();
		//parse_logic(dest);
		parse_expression(dest);
		if (dest->type == TREGISTER)
			reg_free(dest->value);
		else
			reg_free(dest->off);
	}*/
	/*else
		assert(0);*/
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


		if (current_scope != parent)
		{
			var.type = TREGISTER;
			var.value = reg_alloc();
			add_variable(var, current_scope);
			parse_assignment(&var);
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

				//parse_assignment
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
					var.data_type += 8;
				}
				else
					add_bss(var.id, var.data_type, size, flags & 0x1);
				read_token();
			}
			add_variable(var, current_scope);
		}
		//parse_assignment(&var);
		//unread_token();
	}
	else if (token.class == TFUNCTION)
	{
		if (current_scope != parent)
			assert(0);
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
			func = create_function(token.id, vars, 0, sizeof_data(data_type));

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
					var.id = token.id;
					//var.value = offset;
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
						offset++;
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

				current_scope = add_scope(parent, NULL, NULL, NULL);
			        REGISTER reg;
				for (int i = 0; i < func->var_count; i++)
				{
				        reg = reg_alloc();
					//POP(reg, sizeof_data(func->vars[i].data_type));
					MOV_R64OFF(reg, func->vars[i].off, func->vars[i].off_type,
						   func->vars[i].base_ptr, sizeof_data(func->vars[i].data_type));
					func->vars[i].type = TREGISTER;
					func->vars[i].value = reg;
					add_variable(func->vars[i], current_scope);
				}
				if (!strcmp(func->name, "_start"))
					func_prolog();
				//parse_scope();
				/*TODO: parse_scope sucks; make parse_statement except
				 it has while (token.class != '}' instead of TEOF)*/
				parse_statement('}');
				for (int i = 0; i < func->var_count; i++)
				{
					reg_free(func->vars[i].value);
				}
				//writec(9);
				if (func->type == VOID && strcmp(func->name, "_start"))
					func_epilog();
				else if (!strcmp(func->name, "_start"))
					EXIT();
				free_scope(current_scope);
				current_scope = parent;
			}
		}
		read_token();
	}
	//read_token();
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
	parse_logic(&dest);
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
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		//parse_scope();
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
			current_scope = add_scope(current_scope, NULL, NULL, NULL);
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
				{//THE ELSE WILL BE OUTSIDE OF THE IF BLOCK RETARD!!!!!!!!!!!
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
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		read_token();
		assert(token.class == '(');
		OPERAND dest;
		dest.type = TREGISTER;
		dest.value = reg_alloc();

		read_token();
		//LOOK AT THIS AGAIN
		dest.data_type = UINT64_T;
		parse_logic(&dest);

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
		read_token();
		parse_statement(';');
		read_token();

		char loop[100];
		char loop_end[100];
		sprintf(loop, "_loop%d", routine);
		sprintf(loop_end, "_loop_end%d", routine++);
		write_str(loop, SECT_CODE);
		write_strn(":\n", 2, SECT_CODE);
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		OPERAND dest;
		dest.type = TREGISTER;
		dest.value = reg_alloc();
		dest.data_type = UINT64_T;

		parse_logic(&dest);

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

/*static void parse_scope(void)
{
	read_token();
	while (token.class != '}')
	{
		if (token.class == TKEYWORD && token.value == UINT64_T)
			parse_declaration();
		read_token();
	}
	free_scope(current_scope->child);
	}*/

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
		//read_token();
		if (token.class == TKEYWORD && ((token.value >= UINT64_T && token.value <= UINT8_PTR_T) || token.value == VOID))
			parse_declaration(0);
		else if (token.class == TKEYWORD && token.value == EXTERN)
		{
			assert(current_scope == parent);
			write_strn("extern ", 6, SECT_CODE);
			read_token();
			assert(token.class == TKEYWORD);
			parse_declaration(EXTERN_LINK);
			/*assert(token.class == TKEYWORD);
			const int data_type = token.value;
			read_token();
			assert(token.class == TIDENTIFIER);
			writec(' ', SECT_CODE);
			write_str(token.id, SECT_CODE);
			writec(10, SECT_CODE);
			OPERAND tmp_extern = {TSEG_BSS, data_type, token.id, -1};
			add_variable(tmp_extern, current_scope);
			read_token();
			assert(token.class == ';');*/
		}
		else if (token.class == TKEYWORD && token.value == STATIC)
		{
			assert(current_scope == parent);
			read_token();
			assert(token.class == TKEYWORD);
			parse_declaration(0);
		}
		else if (token.class == TIDENTIFIER)
		{
			OPERAND *var = find_var(current_scope, token.id);
			assert(var);
			OPERAND tmp;
			if (PTR(var->data_type))
			{
				tmp.type = TREGISTER;
				tmp.data_type = var->data_type;
				tmp.value = reg_alloc();
				parse_assignment(&tmp);
			}
			else
				parse_assignment(var);
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
				OPERAND ret;
				if (token.class == TIDENTIFIER)
				{
					ret = *find_var(current_scope, token.id);
				}
				/*
				When a function returns a value,
				the value is always placed in RAX
				*/
				OPERAND rax;
				rax.type = TREGISTER;
				rax.value = RAX;
				rax.data_type = ret.data_type;
				MOVE(rax, ret);
				writec(9, SECT_CODE);
				//write_strn("RET\n", 4);
				func_epilog();
			}
			read_token();
		}
		else if (token.class == TFUNCTION)
		{
			/*char *func_name = token.id;
			read_token();
			while (token.class != ')')
			{
				if (token.class == TIDENTIFIER)
				{
					VARIABLE var;//TODO: change type of find_var to VARIABLE*
					var.op = *(find_var(current_scope, token.id));
					PUSH(var.op.value);
				}
				read_token();
			}
			CALL(func_name);*/

			call_function(token.id);

			read_token();
		}
		else if (token.class == '{')
		{
			current_scope = add_scope(current_scope, NULL, NULL, NULL);
			parse_statement('}');
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
			read_token();
		}
		else if (token.class == TKEYWORD && token.value == BREAK)
		{
			JMP(scope_end);
		}
		else if (token.class == TKEYWORD && token.value == CONTINUE)
		{
			JMP(scope_start);
		}
		else if (token.class == ';' || (token.class == TOPERATOR && token.value == '*'))
			read_token();
		/*else if (token.class == '}')
		{

			current_scope = current_scope->parent;
			free_scope(current_scope->child);
		}*/
		//read_token();
	} while (token.class != stop && token.class != TEOF);
	//free_whole_scope(parent);
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
}

static SCOPE* add_scope(SCOPE *parent, SCOPE *child, SCOPE *prev, SCOPE *next)
{
	SCOPE *new = (SCOPE*)malloc(sizeof(SCOPE));
	new->parent = parent;
	new->child = child;
	new->prev = prev;
	new->next = next;

	if (parent)
		new->parent->child = new;

	new->var_length = 0;
	new->var_index = 0;
	new->vars = NULL/*(VARIABLE*)malloc(sizeof(VARIABLE) * 5)*/;

	return new;
}

static void free_scope(SCOPE *scope)
{
	if (scope == NULL)
		return;

	for (int i = 0; i < scope->var_length && i < scope->var_index; i++)
	{
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
	parent = add_scope(NULL, NULL, NULL, NULL);
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
		func_list.funcs = realloc(func_list.funcs, func_list.length + 1);
	func_list.length++;

	func_list.funcs[func_list.index++] = func;
}

static FUNCTION* create_function(char *name, OPERAND *vars, int var_count, int type)
{
	FUNCTION *function = (FUNCTION*)malloc(sizeof(FUNCTION));
	function->name = name;
	function->vars = vars;
	function->type = type;
	function->var_count = var_count;

	return function;
}

static FUNCTION* find_function(char *name)
{
	for (int i = 0; i < func_list.length; i++)
	{
		if (!strcmp(name, func_list.funcs[i]->name))
			return func_list.funcs[i];
	}
	return NULL;
}

static void free_functions(void)
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
		func->vars = realloc(func->vars, func->var_count + 1);
	func->vars[func->var_count] = var;
	func->var_count++;
}

static void call_function(char *func_name)
{
	read_token();
        FUNCTION func = *find_function(func_name);
        OPERAND vars[func.var_count];
	int var_index = 0;
	while (token.class != ')')
	{
		if (token.class == TIDENTIFIER)
		{
			OPERAND *var;
			var = (find_var(current_scope, token.id));
			PUSH(var->value, sizeof_data(var->data_type));
			vars[var_index].data_type = var->data_type;
			vars[var_index].value = var->value;
			var_index++;
			//TODO: add stuff for non-registers
		}
		read_token();
	}
	CALL(func_name);
	for (int i = var_index-1; i >= 0; i--)
	{
		POP(vars[i].value, sizeof_data(vars[i].data_type));
	}
	//read_token();
}

