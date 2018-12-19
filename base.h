#include "type.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <cstring>
#include <set>
#include <algorithm>
#ifndef BASE__32
#define BASE__32

typedef unsigned int SYM_TYPE;
typedef unsigned int TYPE;
typedef unsigned int SYM_NAME;
typedef int INTEGER;
typedef double REAL;
typedef char * STR_NAME;
typedef void * ADDR;
typedef struct
{
	ADDR address;
	TYPE type;
} VAR_VALUE;

typedef union
{
	SYM_NAME sym_name;
	STR_NAME var_name;
	STR_NAME str_name;
	INTEGER int_value;
	REAL real_value;
	STR_NAME terminator_name;
	STR_NAME unterminator_name;
} SYM_VALUE;

typedef struct
{
	SYM_TYPE type;
	SYM_VALUE value;
}token;

token create_str_token(SYM_TYPE type, const char * name){
    token result;
    result.type = type;
    result.value.str_name = new char[strlen(name) + 1];
    strcpy(result.value.str_name, name);
    return result;
}
std::ostream & operator << (std::ostream & os, const token & tk){
	switch (tk.type){
		case VARNAME:
			os << "VARNAME: " << tk.value.var_name;
			break;
		case STRING:
			os << "STRING: " << tk.value.var_name;
			break;
		case UNTERMINATOR:
			os << "UNTERMINATOR: " << tk.value.var_name;
			break;
        case TERMINATOR:
            os << "TERMINATOR: " << tk.value.var_name;
			break;
		case TYPENAME:
			os << "TYPENAME: " << tk.value.sym_name;
			break;
		case OPERATOR:
			os << "OPERATOR: " << tk.value.sym_name;
			break;
		case CONTROLLER:
			os << "CONTROLLER: " << tk.value.sym_name;
			break;
		case DELIMITER:
			os << "DELIMITER: " << tk.value.sym_name;
			break;
		case INT_NUM:
			os << "INT_NUM: " << tk.value.int_value ;
			break;
		case REAL_NUM:
			os << "REAL_NUM: " << tk.value.real_value ;
			break;
        case ABSTRACT_NUM:
            os << "ABSTRACT_NUM";
            break;
        case ABSTRACT_VAR:
            os << "ABSTRACT_VAR";
            break;
		default:
			os << "Cannot print";
	}
	return os;
}
typedef std::unordered_map<std::string, VAR_VALUE*> SYMBOL_TABLE;
bool operator < (const token & tk ,const token & tk1){
    if (tk.type < tk1.type) return true;
    else if (tk.type > tk1.type) return false;
    switch (tk.type){ /* Without sense */
		case VARNAME:
		case STRING:
		case UNTERMINATOR:
        case TERMINATOR:
			return strcmp(tk.value.var_name, tk1.value.var_name) < 0;
		case TYPENAME:
		case OPERATOR:
		case CONTROLLER:
		case DELIMITER:
			return tk.value.sym_name < tk1.value.sym_name;
		case INT_NUM:
			return tk.value.int_value < tk1.value.int_value;
		case REAL_NUM:
			return tk.value.real_value < tk1.value.real_value;
		default:
			return false;
	}
}
bool operator == (const token & tk, const token & tk1){
    return !(tk < tk1) && !(tk1 < tk);
}
bool operator != (const token & tk, const token & tk1){
    return !(tk == tk1);
}

/* There are grammars (generators) */
typedef std::vector<token> word_seq;
typedef struct {
	token before_word;
	std::vector<token> after_words;
} grammar;
bool operator == (const grammar & g1, const grammar & g2){
	return g1.before_word == g2.before_word &&
		g1.after_words == g2.after_words;
}

/* Yacc inner terminators */
const char * var_str = "var";
const char * num_str = "num";
const char * eof_line_str = "#";
const char * spc_str = "space";
const char * begin_str = "$$$";
const char * print_str = "printf";
const char * input_str = "scanf";

/* Now there is a token for example. */
token ERR_TOKEN{ ERR_TYPE, 0xff };
token SPC_TOKEN = create_str_token(TERMINATOR, spc_str);
token BEGIN_TOKEN = create_str_token(UNTERMINATOR, begin_str);
token EOFLINE_TOKEN = create_str_token(TERMINATOR, eof_line_str);
token MAIN_TOKEN = create_str_token(TERMINATOR, "main");
token ABSTRACT_NUM_TOKEN = {ABSTRACT_NUM, 0};
token ABSTRACT_VAR_TOKEN = {ABSTRACT_VAR, 0};

grammar EOF_GRAMMAR{EOFLINE_TOKEN, std::vector<token>()};
grammar ERR_GRAMMAR{ERR_TOKEN, std::vector<token>()};

/* Other yacc types */
typedef std::vector<token> receivers;
typedef std::set<token> first_set;
typedef struct { char action; int index; } movement;
typedef std::map<token, std::vector<movement> > table_item;
typedef struct {
	int grammar_index;
	int pointer;
	token forward_word;
} project;
bool operator <(const project & p1, const project & p2){
	if (p1.grammar_index > p2.grammar_index) return true;
	else if (p1.grammar_index < p2.grammar_index) return false;
	if (p1.pointer > p2.pointer) return true;
	else if (p1.pointer < p2.pointer) return false;
	if (p1.forward_word < p2.forward_word) return true;
	return false;
}

bool operator == (const project & p1, const project & p2){
	return p1.grammar_index == p2.grammar_index &&
		p1.pointer == p2.pointer &&
		p1.forward_word == p2.forward_word;
}

typedef struct {
	std::vector<project> projects;
	std::map<token, int> convert_to; /* Accept a token, and convert to the statement index */
} statement;

bool operator == (statement & p1, statement & p2) {
	std::sort(p1.projects.begin(), p1.projects.end());
	std::sort(p2.projects.begin(), p2.projects.end());
	return p1.projects == p2.projects;
}
/* Exceptions
typedef struct {} invalid_start_word;
typedef struct { int index; int pointer; } invalid_2nd_grammar;
typedef struct { int statement_index; table_item error_item; } ambiguous_2nd_grammar;
typedef struct { int index; } invalid_input_string;
typedef struct { int index; std::string key; } wrong_table_item;
*/

/* Some base functions */
bool is_sep(char x) { return x == ' ' || x == '\t' || x == '\f'; };
bool is_end(char x) { return x == '\n' || x == '\0'; }
bool is_alpha(char x) { return isalpha(x); }
template <class T>
std::ostream & operator << (std::ostream & os, const std::vector<T> & arr){
    os << "[";
	for (auto it : arr) {
		os << it << ",";
	}
	os << "]";
	return os;
}
#endif
