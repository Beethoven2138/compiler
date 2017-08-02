#include <lex.h>

TOKEN prev_token;

void read_token()
{
	char c;
	prev_token = token;
	prev_index = fin->buff->index;
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
	case '[':
	case ']':
	case ',':
	{
		token.class = c;
		token.value = c;
		break;
	}
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
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
		{
			unreadc();
			token.class = TOPERATOR;
			token.value = c;
		}
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
			token.class = TOPERATOR;
			token.value = LESS;
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
			token.class = TOPERATOR;
			token.value = GREATER;
		}
		break;
	}
	case 'u':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("uint64_t", str, 8))
		{
			token.class = TKEYWORD;
			token.value = UINT64_T;
			read_token();
			if (token.value == '*')
			{
				token.value = UINT64_PTR_T;
				token.class = TKEYWORD;
			}
			else
			{
				unread_token();
				token.class = TKEYWORD;
				token.value = UINT64_T;
			}
		}
		else if (!strncmp("uint32_t", str, 8))
		{
			token.class = TKEYWORD;
			token.value = UINT32_T;
			read_token();
			if (token.value == '*')
			{
				token.value = UINT32_PTR_T;
				token.class = TKEYWORD;
			}
			else
			{
				unread_token();
				token.class = TKEYWORD;
				token.value = UINT32_T;
			}
		}
		else if (!strncmp("uint16_t", str, 8))
		{
			token.class = TKEYWORD;
			token.value = UINT16_T;
			read_token();
			if (token.value == '*')
			{
				token.value = UINT16_PTR_T;
				token.class = TKEYWORD;
			}
			else
			{
				unread_token();
				token.class = TKEYWORD;
				token.value = UINT16_T;
			}
		}
		else if (!strncmp("uint8_t", str, 7))
		{
			token.class = TKEYWORD;
			token.value = UINT8_T;
			read_token();
			if (token.value == '*')
			{
				token.value = UINT8_PTR_T;
				token.class = TKEYWORD;
			}
			else
			{
				unread_token();
				token.class = TKEYWORD;
				token.value = UINT8_T;
			}
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'r':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("return", str, 6))
		{
			token.class = TKEYWORD;
			token.value = RETURN;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'b':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("break", str, 5))
		{
			token.class = TKEYWORD;
			token.value = BREAK;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'c':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("continue", str, 8))
		{
			token.class = TKEYWORD;
			token.value = CONTINUE;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'i':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("if", str, 2))
		{
			token.class = TKEYWORD;
			token.value = IF;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'f':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("for", str, 3))//returns 0 on success
		{
			token.class = TKEYWORD;
			token.value = FOR;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'w':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("while", str, 5))
		{
			token.class = TKEYWORD;
			token.value = WHILE;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'e':
	{
		char *str = read_word(fin->buff->index-1);
		if(!strncmp("else", str, 4))
		{
			token.class = TKEYWORD;
			token.value = ELSE;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'm':
	case 'a':
	case 'd':
	case 'h':
	case 'j':
	case 'k':
	case 'o':
	case 'p':
	case 'q':
	case 't':
	case 'x':
	case 'y':
	case 'z':
	case '_':
	{
		char *str = read_word(fin->buff->index-1);
		if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 's':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("sizeof", str, 6))
		{
			token.class = TKEYWORD;
			token.value = SIZEOF;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
		}
		break;
	}
	case 'v':
	{
		char *str = read_word(fin->buff->index-1);
		if (!strncmp("void", str, 4))
		{
			token.class = TKEYWORD;
			token.value = VOID;
		}
		else if (readc() != '(')
		{
			token.class = TIDENTIFIER;
			token.id = str;
			unreadc();
		}
		else
		{
			token.class = TFUNCTION;
			token.id = str;
			unreadc();
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
	fin->buff->index = prev_index;
}
