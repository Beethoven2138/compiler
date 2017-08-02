#include <file.h>

File *make_file(char *name)
{
	assert(name);
	File *file = (File*)malloc(sizeof(File));
	file->name = name;
	file->file = fopen(name, "r");
	file->st = (struct stat*)malloc(sizeof(struct stat));
	stat(file->name, file->st);
	file->buff = (BUFFER*)malloc(sizeof(BUFFER));
	file->buff->text = malloc(file->st->st_size);
	file->buff->index = 0;
	file->buff->str_len = file->st->st_size;

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
			file->buff->text[file->buff->index] = 0;//null-termination
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
				file->buff->text[file->buff->index++] = '\n';
				new_line = false;
			}
			if (white_space)
			{
				file->buff->text[file->buff->index++] = ' ';
				white_space = false;
			}
			file->buff->text[file->buff->index++] = c;

			if (file->buff->index >= file->buff->str_len)
			{
				file->buff->text = realloc(file->buff->text, file->buff->str_len += 5);
				assert(file->buff->text);
			}
		}
	}
	file->buff->str_len = file->buff->index + 1;
	file->buff->index = 0;
	//fclose(file->file);
	return file;
}

char readc(void)
{
	if (fin->buff->index < fin->buff->str_len && fin->buff->text[fin->buff->index] != 0)
		return fin->buff->text[fin->buff->index++];
	else
		return EOF;
}

void unreadc(void)
{
	fin->buff->index--;
}

char* read_word(int index)
{
	char *str = (char*)malloc(sizeof(char) * 20);
	char c;
	int i;
	for (i = 0; i < 20; i++)
	{
		c = fin->buff->text[index++];
		if (c == ' ' || c == '\n' || c == '\r' || c == EOF || c == ';' || c == '(' || c == ')'
		    || c == ']' || c == '[' || c == ',')
		//TODO: Finish this
		{
			fin->buff->index = index-1;
			str[i] = 0;
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

void writec(char c, int section)
{
	if (fout->buff[section].index == fout->buff[section].str_len)
		fout->buff[section].text = realloc(fout->buff[section].text, ++fout->buff[section].str_len + 1);

	//char *text = fout->buff[section].text;
	//fout->buff[section].text[fout->buff[section].index] = c;
/*
	if (fout->buff[section].index != 0)
		fout->buff[section].text = (char*)realloc(fout->buff[section].text, ++(fout->buff[section].index) + 1);
	fout->buff[section].text[fout->buff[section].index] = c;
*/
	fout->buff[section].text[fout->buff[section].index++] = c;
}

void write_strn(const char *str, int len, int section)
{
	assert(str);
	char *text = fout->buff[section].text;
	fout->buff[section].text = (char*)realloc(fout->buff[section].text, len + fout->buff[section].index);
	for (int i = 0; i < len; i++)
	{
		writec(str[i], section);
		//fout->buff[section].text[fout->buff[section].index++] = *(str++);
	}
	//fout->buff[section].text[fout->buff[section].index++]++;
}

void write_str(const char *str, int section)
{
	assert(str);
	for (int i = 0; i < strlen(str); i++)
	{
		writec(str[i], section);
	}
}
