#include <stdlib.h>

#include <file.h>
#include <lex.h>
#include <parse.h>
#include <asm.h>

File *fin;
TOKEN token;

int main(int argc, char *argv[])
{
	if (argc < 2)
		return 0;
	fin = make_file(argv[1]);

	assert(fin);

	init_registers("testasm.asm");

	init_scope();

	parse_statement(TEOF);
	free_functions();

	write_to_file();

	free(fin);
	free(fout);

	return 0;
}
