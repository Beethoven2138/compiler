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
		if (src.type == TSEG_DATA || src.type == TSEG_BSS)
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
	if (token.class == TOPERATOR && token.value == '*')
	{
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
			parse_factor(&operand);
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
			parse_factor(&operand);
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
			if (operand.type == TREGISTER && dest->type == TREGISTER)
				ADD_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
			/*else if (operand.type == TREGISTER && dest->type == TOFFSET)
			{
//void MOV_R64OFF(int dest, int off, int off_type, char *base_ptr, int size);
				REGISTER tmp = reg_alloc();
				MOV_R64OFF(tmp, dest->off, dest->off_type, dest->base_ptr, sizeof_data(dest->data_type));
				ADD_R64R64(tmp, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
				MOV_OFFR64(dest->off, dest->off_type, dest->base_ptr, tmp, sizeof_data(dest->data_type));
				}*/
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
			if (operand.type == TREGISTER && dest->type == TREGISTER)
				SUB_R64R64(dest->value, operand.value, sizeof_data(dest->data_type), sizeof_data(operand.data_type));
			else if (operand.type == TREGISTER && dest->type != TREGISTER)
			{
				OPERAND tmp; tmp.type = TREGISTER; tmp.value = reg_alloc(); tmp.data_type = dest->data_type;
				//MOV_R64OFF(tmp, dest->off, dest->off_type, dest->base_ptr, sizeof_data(dest->data_type));
				MOVE(tmp, *dest);
				SUB_R64R64(tmp.value, operand.value, sizeof_data(dest->data_type), sizeof_data(dest->data_type));
				//MOV_OFFR64(dest->off, dest->off_type, dest->base_ptr, tmp, sizeof_data(dest->data_type));
				MOVE(*dest, tmp);
				reg_free(tmp.value);
			}
			else
				assert(0);
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
	unread_token();
	if (!(token.class == TOPERATOR && token.value == '*'))
	{
		read_token();
		parse_factor(dest);
		read_token();
		OPERAND tmp;
		tmp.type = TREGISTER;
		tmp.value = reg_alloc();
		tmp.data_type = dest->data_type;
		parse_shift(&tmp/*dest*/);
		MOVE(*dest, tmp);
	}
	else
	{
		read_token();
		dest->data_type -= 8;
		parse_expression(dest);
		dest->type = TOFFSET;
		dest->off = dest->value;
		dest->off_type = TREGISTER;
		dest->base_ptr = "0";
		read_token();
		parse_expression(dest);
		reg_free(dest->off);
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
			/* Only for when there's the register keyword*/
			if (flags & 64)
			{
				var.type = TREGISTER;
				var.value = reg_alloc();
			}
			else
			{
				var.base_ptr = spec_registers[RBP];
				var.type = TOFFSET;
				var.off = current_scope->offset;
				var.off_type = TIMMEDIATE;
			}
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

				current_scope = add_scope(parent, NULL, NULL, NULL);
				//if (strcmp(func->name, "main"))
					func_prolog();
			        REGISTER reg;
				for (int i = 0; i < func->var_count; i++)
				{
					//Don't think this is necessary
				        /*reg = reg_alloc();
					//POP(reg, sizeof_data(func->vars[i].data_type));
					MOV_R64OFF(reg, func->vars[i].off, func->vars[i].off_type,
						   func->vars[i].base_ptr, sizeof_data(func->vars[i].data_type));
					func->vars[i].type = TREGISTER;
					func->vars[i].value = reg;*/
					add_variable(func->vars[i], current_scope);
				}

				//parse_scope();
				/*TODO: parse_scope sucks; make parse_statement except
				 it has while (token.class != '}' instead of TEOF)*/
				parse_statement('}');
				for (int i = 0; i < func->var_count; i++)
				{
					reg_free(func->vars[i].value);
				}
				//writec(9);
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
		unsigned int offset = current_scope->offset;
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
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
			current_scope = add_scope(current_scope, NULL, NULL, NULL);
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
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		current_scope->offset = offset;
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
			//EXPERIMENTAL!!!
			unsigned int offset = current_scope->offset;
			current_scope = add_scope(current_scope, NULL, NULL, NULL);
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
		read_token();
		parse_statement(';');
		read_token();

		char loop[100];
		char loop_end[100];
		sprintf(loop, "_loop%d", routine);
		sprintf(loop_end, "_loop_end%d", routine++);
		write_str(loop, SECT_CODE);
		write_strn(":\n", 2, SECT_CODE);
		unsigned int offset = current_scope->offset;
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		current_scope->offset = offset;
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
		//EXPERIMENTAL!!!
	        offset = current_scope->offset;
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		current_scope->offset = offset;

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
		//read_token();
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
			/*OPERAND *var;
			read_token();read_token();
			unsigned int data_type = 0;
			while (!(token.class == '}' && token.class == '}'))
			{
				assert(token.class == TKEYWORD && token.value != STRUCT);
				if (!var)
				{
					var = (OPERAND*)malloc(sizeof(OPERAND));
				        add_variable(var, current_scope);
				}
				else
				{
					var->next = (OPERAND*)malloc(sizeof(OPERAND));
					var = var->next;
				}
				var->next = NULL;
				var->data_type = token.value;
				read_token();
				assert(token.class == TIDENTIFIER);
				var->id = token.id;
				read_token();
				}*/
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
				func_epilog();
			}
			read_token();
		}
		else if (token.class == TFUNCTION)
		{
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
	//CHECK THIS!!!
	new->offset = 16;
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
	//ensure that the data type is set before calling this function
	func->offset += sizeof_data(var.data_type);
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
}
