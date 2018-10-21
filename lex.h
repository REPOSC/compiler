#include "InputStream.h"
#include "base.h"
#include "state.h"

#ifndef LEX__32
#define LEX__32

typedef struct 
{
	InputStream m_istream;
	State m_state;
} Lex;

Lex CreateLex(const char * filename)
{
	Lex result;
	result.m_istream = CreateStream(filename);
	return result;
}

token GetNextToken(Lex * lex)
{
	while (check_if_blank(PeekFromStream(&lex->m_istream)))
		GetFromStream(&lex->m_istream);
	char peek_char = PeekFromStream(&lex->m_istream);
	printf("%c\n", peek_char);
	if (peek_char <= 0)
		return EOF_TOKEN;
	else if (check_if_leu(peek_char))
		return (lex->m_state).get_keyword(&lex->m_istream);
	else if (check_if_number(peek_char))
		return (lex->m_state).get_number(&lex->m_istream);
	else
		return (lex->m_state).get_symbol(&lex->m_istream);
}
#endif
