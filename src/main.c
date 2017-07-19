#include <stdlib.h>

#include <file.h>
#include <lex.h>
#include <parse.h>
#include <asm.h>

File *fin;
TOKEN token;

int main(int argc, char *argv[])
{
	fin = make_file(argv[1]);

	//read_token();

	init_registers("testasm.asm");

	init_scope();

	parse_statement();

	write_to_file();

	free(fin);
	free(fout);

	return 0;
}
