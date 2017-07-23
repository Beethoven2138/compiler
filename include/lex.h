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

	VOID,
	CHAR,
	SHORT,
};//Data types

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

void read_token(void);
void unread_token();

#endif
