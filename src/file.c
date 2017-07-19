#include <file.h>

File *make_file(char *name)
{
	assert(name);
	File *file = (File*)malloc(sizeof(File));
	file->name = name;
	file->file = fopen(name, "r");
	file->st = (struct stat*)malloc(sizeof(struct stat));
	stat(file->name, file->st);
	file->text = malloc(file->st->st_size);
	file->str_len = file->st->st_size;

	int c;
	bool white_space = false;
	bool new_line = false;
	while (1)
	{
		c = fgetc(file->file);
		if (c == '\n' || c == '\r')
		{
			new_line = true;
			continue;
		}
		if (c == EOF)
		{
			file->text[file->index] = 0;//null-termination
			break;
		}
		if (c == ' ' || c == 9/*tab*/)
		{
			white_space = true;
			continue;
		}

		else
		{
			if (new_line)
			{
				file->text[file->index++] = '\n';
				new_line = false;
			}
			if (white_space)
			{
				file->text[file->index++] = ' ';
				white_space = false;
			}
			file->text[file->index++] = c;

			if (file->index >= file->str_len)
			{
				file->text = realloc(file->text, file->str_len += 5);
				assert(file->text);
			}
		}
	}
	file->str_len = file->index + 1;
	file->index = 0;
	//fclose(file->file);
}

char readc(void)
{
	if (fin->index < fin->str_len && fin->text[fin->index] != 0)
		return fin->text[fin->index++];
	else
		return EOF;
}

void unreadc(void)
{
	fin->index--;
}

char* read_word(int index)
{
	char *str = (char*)malloc(sizeof(char) * 20);
	char c;
	int i;
	for (i = 0; i < 20; i++)
	{
		c = fin->text[index++];
		if (c == ' ' || c == '\n' || c == '\r' || c == EOF || c == ';'/* || c == ']'
		    || c == '[' || c == '}' || c == '{' || c == '|' || c == '\' || c == '&'*/)
		//TODO: Finish this
		{
			fin->index = index-1;
			break;
		}
		str[i] = c;
	}
	if (i < 19)
	{
		str[i+1] = 0;
	}
	return str;
}

void writec(char c)
{
	if (fout->index == fout->str_len)
		fout->text = realloc(fout->text, ++fout->str_len + 1/*NULL-terminator byte :D*/);

	fout->text[fout->index++] = c;
}

void write_strn(const char *str, int len)
{
	assert(str);
	for (int i = 0; i < len; i++)
	{
		writec(str[i]);
	}
}

void write_str(const char *str)
{
	assert(str);
	for (int i = 0; i < strlen(str); i++)
	{
		writec(str[i]);
	}
}
