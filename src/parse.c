#include <parse.h>
#include <lex.h>
#include <file.h>

static void MOVE(OPERAND dest, OPERAND src)
{
	if (dest.type == src.type && dest.value == src.value)
		return;

	if (dest.type == TIMMEDIATE)
		assert(0);
	else if (dest.type == TREGISTER)
	{
		if (src.type == TIMMEDIATE)
			MOV_R64I(dest.value, src.value);
		else if (src.type == TREGISTER)
			MOV_R64R64(dest.value, src.value);
		else if (src.type == TOFFSET)
			MOV_R64OFF(dest.value, src.value);
	}
	else if (dest.type == TOFFSET)
	{
		if (src.type == TIMMEDIATE)
			MOV_OFFI(dest.value, src.value);
		else if (src.type == TREGISTER)
			MOV_OFFR64(dest.value, src.value);
		else if (src.type == TOFFSET)
		{
			/*x86 doesn't allow for MOV addr, addr
			  so src needs to be moved to a register first
			*/
			REGISTER tmp = reg_alloc();
			MOV_R64OFF(tmp, src.value);
			MOV_OFFR64(dest.value, tmp);
			reg_free(tmp);
		}
	}
}

static void parse_factor(OPERAND *dest)
{
	static int routine = 0;

	if (token.class != '(')
	{
		OPERAND src;
		if (token.class == TNUMBER)
		{
			src.type = TIMMEDIATE;
			src.value = token.value;
		}
		else if (token.class == TIDENTIFIER)
		{
			src = *find_var(current_scope, token.id);
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
		read_token();

		parse_expression(&operand);
		MOV_R64R64(dest->value, operand.value);
		read_token();
		reg_free(operand.value);
	}
}

static void parse_term(OPERAND *dest)
{
	parse_factor(dest);

	while (token.class == TOPERATOR && (token.value == '*' || token.value == '/' || token.value == '%'))
	{
		if (token.value == '*')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_factor(&operand);
			MOV_R64R64(RAX, dest->value);
			MUL_R64(operand.value);
			MOV_R64R64(dest->value, RAX);
			reg_free(operand.value);
		}
		else if (token.value == '/')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_factor(&operand);
			MOV_R64R64(RAX, dest->value);
			DIV_R64(operand.value);
			MOV_R64R64(dest->value, RAX);
			reg_free(operand.value);
		}
		else if (token.value == '%')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_factor(&operand);
			MOV_R64R64(RAX, dest->value);
			DIV_R64(operand.value);
			//The remainder of a division is stored in RDX
			MOV_R64R64(dest->value, RDX);
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
			parse_term(&operand);
			ADD_R64R64(dest->value, operand.value);
			reg_free(operand.value);
		}
		else if (token.value == '-')
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_term(&operand);
			SUB_R64R64(dest->value, operand.value);
			reg_free(operand.value);
		}
	}
}


static void parse_relation(OPERAND *dest)
{
	parse_expression(dest);

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
			parse_expression(&operand);
			CMP(dest->value, operand);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JNE(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
		else if (token.value == NOT_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_expression(&operand);
			CMP(dest->value, operand);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JE(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
		else if (token.value == GREATER)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_expression(&operand);
			CMP(dest->value, operand);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JB(else_routine);//TODO: add in JL for signed values;
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
		else if (token.value == LESS)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_expression(&operand);
			CMP(dest->value, operand);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JA(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
		else if (token.value == LESS_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_expression(&operand);
			CMP(dest->value, operand);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JA(else_routine);
			JNE(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
		else if (token.value == GREATER_EQUAL)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_expression(&operand);
			CMP(dest->value, operand);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rif_%d", routine);
			sprintf(end_routine, "_rend_%d", routine);
			routine++;
			JB(else_routine);
			JNE(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
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
			parse_relation(&operand);
			AND_R64R64(dest->value, operand.value);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rlog_%d", routine);
			sprintf(end_routine, "_rendlog_%d", routine);
			routine++;
			JNE(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
		else if (token.value == OR)
		{
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_relation(&operand);
			OR_R64R64(dest->value, operand.value);
			char else_routine[100];
			char end_routine[100];
			sprintf(else_routine, "_rlog_%d", routine);
			sprintf(end_routine, "_rendlog_%d", routine);
			routine++;
			JNE(else_routine);
			MOV_R64I(dest->value, 1);
			JMP(end_routine);
			write_str(else_routine);
			write_strn(":\n", 2);
			MOV_R64I(dest->value, 0);
			write_str(end_routine);
			write_strn(":\n", 2);
			reg_free(operand.value);
		}
	}
}

static void parse_assignment(OPERAND *dest)
{
	read_token();
	if (token.class == TOPERATOR && token.value == '=')
	{
		read_token();
		//parse_logic(dest);
		parse_expression(dest);
	}
	/*else
		assert(0);*/
}

static void parse_declaration(void)
{
	int type = token.value;

	read_token();

	if (token.class == TIDENTIFIER)
	{
		VARIABLE var;
		var.type = type;
		var.op.type = TREGISTER;
		var.id = token.id;
		var.op.value = reg_alloc();
		add_variable(var, current_scope);

		parse_assignment(&(var.op));
		//unread_token();
	}
	else if (token.class == TFUNCTION)
	{
		if (current_scope != parent)
			assert(0);
		VARIABLE *vars;
		FUNCTION *func;
		func = find_function(token.id);
		if (func != NULL)
		{
			while (token.class == '{')
				read_token();
			goto scope;
		}
		else
		/*This means that the function hasn't already been declared
		  which means that the arguments of the function have to be sorted out
		  If there already was a declaration, this step is skipped*/
		{
			vars = (VARIABLE*)malloc(sizeof(VARIABLE));
			func = create_function(token.id, vars, 0, type);

			func->type = type;

			read_token();

			if (token.value != '(')
				assert(0);
			read_token();
			VARIABLE var;
			int offset = 8;

			while (token.class != ')')
			{
				if (token.class == TKEYWORD)
				{
					var.type = token.value;//CHECK TO SEE IF THEY USE THE SAME ENUM!
				}
				else if (token.class == TIDENTIFIER)
				{
					var.id = token.id;
					var.op.value = offset;
					var.op.type = TOFFSET;
					add_funcvar(func, var);
					var.type = 0;
					var.id = 0;
					offset += 8;//For 8 byte data types eg uint64_t
				}
				else
					assert(0);
				read_token();
			}
			add_function(func);
			read_token();
scope:
			if (token.class == '{')
			{
				write_str(func->name);
				write_strn(":\n", 2);
				func_prolog();
				current_scope = add_scope(parent, NULL, NULL, NULL);
				for (int i = 0; i < func->var_count; i++)
				{
					add_variable(func->vars[i], current_scope);
				}
				//parse_scope();
				/*TODO: parse_scope sucks; make parse_statement except
				 it has while (token.class != '}' instead of TEOF)*/
				parse_statement('}');
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
	//read_token();
}

static void parse_condition(void)
{
	static int routine = 0;



	OPERAND dest;
	dest.type = TREGISTER;
	dest.value = reg_alloc();

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

	CMP(dest.value, tmp);

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

	write_str(else_routine);
	write_strn(":\n", 2);

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

	write_str(end_routine);
	write_strn(":\n", 2);

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
		write_str(loop);
		write_strn(":\n", 2);
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		read_token();
		assert(token.class == '(');
		OPERAND dest;
		dest.type = TREGISTER;
		dest.value = reg_alloc();

		read_token();
		parse_logic(&dest);

		OPERAND tmp;
		tmp.type = TIMMEDIATE;
		tmp.value = 1;

		CMP(dest.value, tmp);
		JNE(loop_end);
		read_token();
		if (token.class == '{')
		{
			parse_statement('}');
			JMP(loop);
			write_str(loop_end);
			write_strn(":\n", 2);
			read_token();
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
		}
		else
			assert(0);
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
		write_str(loop);
		write_strn(":\n", 2);
		current_scope = add_scope(current_scope, NULL, NULL, NULL);
		OPERAND dest;
		dest.type = TREGISTER;
		dest.value = reg_alloc();

		parse_logic(&dest);

		OPERAND tmp;
		tmp.type = TIMMEDIATE;
		tmp.value = 1;

		CMP(dest.value, tmp);
		JNE(loop_end);


		/*
		The final part of the for loop happens at the end of the scope
		therefore the code for it has to be added at the end
		so file_pos is there so that the program can go back to it
		at the end of the scope
		*/
		int file_pos = fin->index;

		read_token();

		while (token.class != '{')
			read_token();

		parse_statement('}');
		int end_loop_pos = fin->index;
		fin->index = file_pos;
		parse_statement(')');
		JMP(loop);
		write_str(loop_end);
		write_strn(":\n", 2);
		fin->index = end_loop_pos;
		read_token();
		current_scope = current_scope->parent;
		free_scope(current_scope->child);
	}
}

static void parse_scope(void)
{
	read_token();
	while (token.class != '}')
	{
		if (token.class == TKEYWORD && token.value == UINT64_T)
			parse_declaration();
		read_token();
	}
	free_scope(current_scope->child);
}

void parse_statement(int stop)
{
	static int scope = 0;
	char scope_start[100];
	char scope_end[100];
	sprintf(scope_start, "_scope_start%d", scope);
	sprintf(scope_end, "_scope_end%d", scope++);
	write_str(scope_start);
	write_strn(":\n", 2);

	int type;
	read_token();
	do
	{
		//read_token();
		if (token.class == TKEYWORD && (token.value == UINT64_T || token.value == VOID))
			parse_declaration();
		else if (token.class == TIDENTIFIER)
		{
			OPERAND *tmp = find_var(current_scope, token.id);
			if (tmp == NULL)
				assert(0);
			parse_assignment(tmp);
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
				MOVE(rax, ret);
				writec(9);
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
		else if (token.class == ';')
			read_token();
		/*else if (token.class == '}')
		{
			
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
		}*/
		//read_token();
		printf("%d", type++);
	} while (token.class != stop && token.class != TEOF);
	//free_whole_scope(parent);
	write_str(scope_end);
	write_strn(":\n", 2);
}

static void add_variable(VARIABLE var, SCOPE *scope)
{
	if (scope->var_length == 0)
	{
		scope->vars = (VARIABLE*)malloc(sizeof(VARIABLE));
		scope->var_length++;
	}
	else if (scope->var_index == scope->var_length)
	{
		scope->vars = (VARIABLE*)realloc(scope->vars, sizeof(VARIABLE) * (scope->var_length+1));
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
		reg_free(scope->vars[i].op.value);
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
			return &(scope.vars[i].op);
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

static FUNCTION* create_function(char *name, VARIABLE *vars, int var_count, int type)
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

static void add_funcvar(FUNCTION *func, VARIABLE var)
{
	if (func->var_count > 0)
		func->vars = realloc(func->vars, func->var_count + 1);
	func->vars[func->var_count] = var;
	func->var_count++;
}

static void call_function(char *func_name)
{
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
	CALL(func_name);
	//read_token();
}

