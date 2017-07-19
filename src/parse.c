#include <parse.h>
#include <lex.h>
#include <file.h>


void parse_factor(OPERAND *dest)
{
	static int routine = 0;


	if (token.class == TNUMBER)
	{
		MOV_R64I(dest->value, token.value);
		//dest->type = TIMMEDIATE;
		//dest->value = token.value;
		read_token();
	}
	else if (token.class == TIDENTIFIER)
	{
		//*dest = find_local_var(*current_scope, token.id);
		//MOV_R64R64(dest->value, find_var(current_scope, token.id));

		OPERAND *var = find_var(current_scope, token.id);
		//if (var->value != dest->value)
		{
			MOV_R64R64(dest->value, var->value);
		}

		//dest->type = TREGISTER;
		//dest->value = find_local_var(*current_scope, token.id);
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

void parse_term(OPERAND *dest)
{
	parse_factor(dest);

	while (token.class == TOPERATOR && (token.value == '*' || token.value == '/' || token.value == '^'))
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
	}
}

/*
If immediate value, don't move it to a register, do addition immediately
If register, same
Only have to move immediate value to a register in MUL and DIV
*/

void parse_expression(OPERAND *dest)
{
	parse_term(dest);

	static int routine = 0;

	while (token.class == TOPERATOR && (token.value == '+' || token.value == '-'))
	{
		if (token.class == TOPERATOR && token.value == AND)
		{//DOESN'T WORK!!!!!!!!!!!!!!!
			read_token();
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			parse_expression(&operand);
			AND_R64R64(dest->value, operand.value);
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
		else if (token.value == '+')
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
	/*if (token.class == TOPERATOR && token.value == EQUAL)
		unread_token();*/
}


void parse_relation(OPERAND *dest)
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
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			read_token();
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
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			read_token();
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
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			read_token();
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
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			read_token();
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
			OPERAND operand;
			operand.type = TREGISTER;
			operand.value = reg_alloc();
			read_token();
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


void parse_logic(OPERAND *dest)
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



void parse_declaration(void)
{
	int type = token.value;

	read_token();

	if (token.class == TIDENTIFIER)
	{
		VARIABLE var;
		var.op.type = type;
		var.id = token.id;
		var.op.value = reg_alloc();
		add_variable(var, current_scope);
		read_token();
		if (token.class == TOPERATOR && token.value == '=')
		{
			read_token();
			OPERAND dest;
			dest.type = TREGISTER;
			dest.value = var.op.value;
			parse_expression(&dest);
		}

		else
		{
			printf("%d", token.class);
			assert(0);
		}
	}
	//read_token();
}

void parse_condition(void)
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
		parse_scope();
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
			parse_scope();
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

void parse_scope(void)
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

void parse_statement(void)
{
	int type;
	read_token();
	while (token.class != TEOF)
	{
		if (token.class == TKEYWORD && token.value == UINT64_T)
			parse_declaration();
		else if (token.class == TKEYWORD && token.value == IF)
		{
			parse_condition();
		}
		else if (token.class == '{')
		{
			current_scope = add_scope(current_scope, NULL, NULL, NULL);
		}
		else if (token.class == '}')
		{
			current_scope = current_scope->parent;
			free_scope(current_scope->child);
		}
		read_token();
		printf("%d", type++);
	}
	free_whole_scope(parent);
}

static void add_variable(VARIABLE var, SCOPE *scope)
{
	if (scope->var_index == scope->var_length)
	{
		scope->vars = (VARIABLE*)realloc(scope->vars, sizeof(VARIABLE) * (scope->var_length+2));
		scope->var_length += 2;
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

	new->var_length = 5;
	new->var_index = 0;
	new->vars = (VARIABLE*)malloc(sizeof(VARIABLE) * 5);

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
	SCOPE *parent = add_scope(NULL, NULL, NULL, NULL);
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

static void add_function(FUNCTION func)
{
	if (func_list.index == func_list.length)
		func_list.funcs = realloc(func_list.funcs, func_list.length + 1);
	func_list.length++;

	func_list.funcs[func_list.index++] = func;
}

static FUNCTION* create_function(char *name, VARIABLE *vars, int type)
{
	FUNCTION *function = (FUNCTION*)malloc(sizeof(FUNCTION));
	function->name = name;
	function->vars = vars;
	function->type = type;

	return function;
}

static FUNCTION* find_function(char *name)
{
	for (int i = 0; i < func_list.length; i++)
	{
		if (!strcmp(name, func_list.funcs[i].name))
			return &(func_list.funcs[i]);
	}
	return NULL;
}

static void free_functions(void)
{
	free(func_list.funcs);
}

static void init_functions(void)
{
	func_list.funcs = 0;
	func_list.length = 0;
	func_list.index = 0;
}
