#ifndef LEX_H
#define LEX_H

#include <stdbool.h>
#include <stdlib.h>

#include <file.h>

enum
{
	TEOF = 0,
	TNUMBER,
	TIDENTIFIER,
	TOPERATOR,
	TKEYWORD,
	TFUNCTION,
};

enum
{
	EQUAL = 128,
	NOT_EQUAL,
	GREATER,
	LESS,
	GREATER_EQUAL,
	LESS_EQUAL,
	AND,
	AND_EQUAL,
	OR,
	OR_EQUAL,
	XOR,
	XOR_EQUAL,
	NOT,
	IF,
	ELSE,
	RETURN,
	WHILE,
	FOR,
	BREAK,
	CONTINUE,
	SIZEOF,
};

enum
{
	UINT64_T,
	UINT32_T,
	UINT16_T,
	UINT8_T,

	SINT64_T,
	SINT32_T,
	SINT16_T,
	SINT8_T,

	UINT64_PTR_T,
	UINT32_PTR_T,
	UINT16_PTR_T,
	UINT8_PTR_T,

	SINT64_PTR_T,
	SINT32_PTR_T,
	SINT16_PTR_T,
	SINT8_PTR_T,

	VOID,
};//Data types


#define PTR(a) ((a >= UINT64_PTR_T && a <= SINT8_PTR_T) ? 1 : 0)



typedef struct
{
	int class;

	bool space;//Space before it
	bool start;//Start of the line

	union
	{
		int value;//Also used as an index into a hash table for keywords
		char *id;
	};
} TOKEN;

extern TOKEN token;
extern TOKEN prev_token;

static int prev_index;


void read_token(void);
void unread_token();

#endif
