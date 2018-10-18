#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

struct InputStream
{
	FILE * file;
	char buffer[BUFFER_SIZE];
	int pointer;
	int max_size;
};

InputStream CreateStream(const char * filename)
{
	InputStream result;
	result.file = fopen(filename, "r");
	result.max_size = fread(result.buffer, sizeof(char), BUFFER_SIZE, result.file);
	result.pointer = 0;
	return result;
}

void ContinueStream(InputStream * stream)
{
	stream->max_size = fread(stream->buffer, sizeof(char), BUFFER_SIZE, stream->file);
	stream->pointer = 0;
}

char GetFromStream(InputStream * stream)
{
	if (stream->pointer >= stream->max_size)
	{
		ContinueStream(stream);
		if (stream->max_size == 0)
		{
			return EOF;
		}
	}
	return stream->buffer[stream->pointer ++];
}