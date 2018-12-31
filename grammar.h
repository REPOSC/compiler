#include "token.h"
#ifndef GRAMMAR__32
#define GRAMMAR__32

typedef std::vector<token> word_seq;

#define makeleaf 1
#define makenode 2
#define same 3
#define null 4

typedef unsigned move_type;

typedef struct {
	token before_word;
	word_seq after_words;
	move_type move;//存放@后面的第一个串，从1-4分别代表： makeleaf  makenode  same  null
	token strange_token;//存放从第二个token
} grammar;
bool operator == (const grammar & g1, const grammar & g2){
	return g1.before_word == g2.before_word &&
		g1.after_words == g2.after_words;
}

grammar EOF_GRAMMAR{EOFLINE_TOKEN, std::vector<token>()};
grammar ERR_GRAMMAR{ERR_TOKEN, std::vector<token>()};

#endif // GRAMMAR__32
	