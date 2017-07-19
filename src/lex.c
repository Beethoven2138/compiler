#include <lex.h>

TOKEN prev_token;

void read_token()
{
	char c;
	prev_token = token;
redo:

	c = readc();
	switch(c)
	{
	case EOF:
	{
		token.class = TEOF;
		token.value = 0;
		break;
	}
	case ' ':
	case '\n':
	case '\r':
		goto redo;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		token.class = TNUMBER;
		token.value = c - '0';
		char tmp = readc();
		while (tmp >= '0' && tmp <= '9')
		{
			token.value *= 10;
			token.value += (tmp - '0');
			tmp = readc();
		}
		unreadc();
		break;
	}
	case '(':
	case ')':
	case ';':
	case '{':
	case '}':
	{
		token.class = c;
		token.value = c;
		break;
	}
	case '+':
	case '-':
	case '*':
	case '/':
	case '^':
	{
		token.class = TOPERATOR;
		token.value = c;
		break;
	}
	case '&':
	{
		if (readc() == '&')
		{
			token.class = TOPERATOR;
			token.value = AND;
		}
		else
			assert(0);
		break;
	}
	case '|':
	{
		if (readc() == '|')
		{
			token.class = TOPERATOR;
			token.value = OR;
		}
		else
			assert(0);
		break;
	}
	case '=':
	{
		if (readc() == '=')
		{
			token.class = TOPERATOR;
			token.value = EQUAL;
		}
		else
		{
			unreadc();
			token.class = TOPERATOR;
			token.value = c;
		}
		break;
	}
	case '!':
	{
		if (readc() == '=')
		{
			token.class = TOPERATOR;
			token.value = NOT_EQUAL;
		}
		else
		{
			assert(0);
			//TODO: include ! operator;
		}
		break;
	}
	case '<':
	{
		if (readc() == '=')
		{
			token.class = TOPERATOR;
			token.value = LESS;
		}//TODO: add bitwise operators
		else
		{
			unreadc();
			token.class = '<';
			token.value = '<';
		}
		break;
	}
	case '>':
	{
		if (readc() == '=')
		{
			token.class = TOPERATOR;
			token.value = GREATER;
		}
		else
		{
			unreadc();
			token.class = '>';
			token.value = '>';
		}
		break;
	}
	case 'u':
	{
		char *str = read_word(fin->index-1);
		if (!strncmp("uint64_t", str, 8))
		{
			token.class = TKEYWORD;
			token.value = UINT64_T;
		}
		else
		{
			token.class = TIDENTIFIER;
			token.id = str;
		}
		break;
	}
	case 'i':
	{
		char *str = read_word(fin->index-1);
		if (!strncmp("if", str, 2))
		{
			token.class = TKEYWORD;
			token.value = IF;
		}
		else
		{
			token.class = TIDENTIFIER;
			token.id = str;
		}
		break;
	}
	case 'e':
	{
		char *str = read_word(fin->index-1);
		if(!strncmp("else", str, 4))
		{
			token.class = TKEYWORD;
			token.value = ELSE;
		}
		else if (str[strlen(str) - 2] != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
		}
		break;
	}
	case 'v':
	{
		char *str = read_word(fin->index-1);
		if (!strncmp("void", str, 4))
		{
			token.class = TKEYWORD;
			token.value = VOID;
		}
		else
		{
			token.class = TIDENTIFIER;
			token.id = str;
		}
		break;
	}
	printf("ERROR: UNKNOWN LEXIM\n");
	assert(0);
	}
}

void unread_token()
{
	token = prev_token;
}
