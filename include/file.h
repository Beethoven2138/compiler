#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* This file is for file I/O. It reads from a file and stores
   the text in a text buffer ignoring the following characters:
   "\n" "\r" and a sequence of n number of whitespace will be 
   converted to " ".
*/

#define SECT_DATA 0
#define SECT_BSS 1
#define SECT_CODE 2

typedef struct
{
	char *text;
	int index;
	int str_len;
} BUFFER;

typedef struct
{
	FILE *file;
	BUFFER *buff;
	char *name;
	//int str_len;

	struct stat *st;
} File;

File *make_file(char *name);

extern File *fin;

extern File *fout;

char readc(void);
void unreadc(void);

char* read_word(int index);

void writec(char c, int section);
void write_strn(const char *str, int len, int section);
void write_str(const char *str, int section);

void write_to_file(void);

#endif
