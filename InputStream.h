#include <stdio.h>
#include <stdlib.h>
#ifndef INPUTSTREAM__32
#define INPUTSTREAM__32

#if _MSC_VER >= 1400
#pragma warning(disable:4996)
#endif

#define BUFFER_SIZE 1024

typedef struct
{
	FILE * file;
	char buffer[BUFFER_SIZE];
	int pointer;
	int max_size;
} InputStream;

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
	char result = stream->buffer[stream->pointer];
	if (result <= 0)
		return EOF;
	else
		stream->pointer++;
	if (stream->pointer > stream->max_size)
	{
		ContinueStream(stream);
	}
	return result;
}

char PeekFromStream(const InputStream * stream)
{
	return stream->buffer[stream->pointer];
}

#endif