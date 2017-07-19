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

typedef struct
{
	FILE *file;
	char *text;
	char *name;
	int index;
	int str_len;

	struct stat *st;
} File;

File *make_file(char *name);

extern File *fin;

extern File *fout;

char readc(void);
void unreadc(void);

char* read_word(int index); 

void writec(char c);
void write_strn(const char *str, int len);
void write_str(const char *str);

void write_to_file(void);

#endif
