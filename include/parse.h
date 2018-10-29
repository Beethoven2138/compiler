#ifndef PARSE_H
#define PARSE_H

#include <lex.h>
#include <file.h>
#include <asm.h>

#define EXTERN_LINK 1

typedef struct _variable
{
	char *id;
	int type;

	OPERAND op;

	//for unions
	//struct _variable *next;
	
	//TODO: add conditions for when all the registers are used up
} VARIABLE;

typedef struct SCOPE
{
	struct SCOPE *parent;
	struct SCOPE *child;

	struct SCOPE *prev;
	struct SCOPE *next;

	int var_length;
	int var_index;
	OPERAND *vars;
	unsigned int offset;
} SCOPE;

typedef struct FUNCTION
{
	char *name;
	OPERAND *vars;
	int var_count;
	int type;
	unsigned int offset;
} FUNCTION;

typedef struct
{
	FUNCTION **funcs;
	int length;
	int index;
} FUNCTION_LIST;



static SCOPE *parent;
static SCOPE *current_scope;

static FUNCTION_LIST func_list;

static void MOVE(OPERAND dest, OPERAND src);

static void parse_factor(OPERAND *dest);
static void parse_prefix(OPERAND *dest);
static void parse_term(OPERAND *dest);
static void parse_expression(OPERAND *dest);
static void parse_shift(OPERAND *dest);
static void parse_relation(OPERAND *dest);
static void parse_logic(OPERAND *dest);
static void parse_assignment(OPERAND *dest);


static void parse_declaration(int flags);
void parse_statement(int stop);

static void parse_scope(void);

static void add_variable(OPERAND var, SCOPE *scope);

static SCOPE* add_scope(SCOPE *parent, SCOPE *child, SCOPE *prev, SCOPE *next);

void init_scope(void);

static void free_scope(SCOPE *scope);
void free_whole_scope(SCOPE *parent);

static OPERAND *find_var(SCOPE *scope, char *id);

static OPERAND *find_var_in_scope(SCOPE scope, char *id);

static void add_function(FUNCTION *func);
static FUNCTION* create_function(char *name, OPERAND *vars, int var_count, int type, unsigned int offset);

static FUNCTION* find_function(char *name);
static void free_functions(void);
static void add_funcvar(FUNCTION *func, OPERAND var);

static void call_function(char *func_name);

static void parse_loop(void);

#endif
