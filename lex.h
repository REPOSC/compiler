#include <fstream>
#include <string>
#include <cstring>
#include <cmath>
#include "type.h"
#include "token.h"
#include "grammar.h"
#include <unordered_set>
#include <unordered_map>
#ifndef LEX__32
#define LEX__32

typedef struct {} File_Error;

#ifdef Lex__DEBUG
#define Symbol() Print_Symbol()
#define Symbol_Token() Print_Symbol_Token()
#else
#define Symbol() _empty()
#define Symbol_Token() _empty()
#endif

class Lex
{
#ifdef Lex__DEBUG
	private:
		std::string debug_str;
	public:
		void Print_Symbol_Token()
		{
			if (debug_str[0] > 0)
			{
				printf("%-15s", debug_str.c_str());
			}
			else
			{
				printf("EOF            ", debug_str.c_str());
			}
			debug_str = "";
		}
		void Print_Symbol()
		{
			if (debug_str == "\n") printf("读取到换行符，忽略\n");
			else if (debug_str == "\r") printf("读取到回车符号，忽略\n");
			else if (debug_str == " ") printf("读取到空格，忽略\n");
			else if (debug_str == "\t") printf("读取到制表符，忽略\n");
			else printf("【%s】，忽略\n", debug_str.c_str());
			debug_str = "";
		}
#else
	public:
		inline void _empty() {/* Do nothing */}
	private:
#endif
	struct Node
	{
		token result_;
		Node * next;
	};
	const static std::string ID_[], ID_MOVE_[], ID_STRANGE_[];
	const static move_type NAME_MOVE_[];
	const static token NAME_[];
	const static char ID1_[], ID2_[], ID3_[], ID4_[];
	const static Node NAME1_[], NAME2_[], NAME3_[], NAME4_[];

private:
	std::unordered_set<std::string> GR_STRANGE_;
	std::unordered_map<std::string, move_type>  GR_MOVE_;
	std::unordered_map<std::string, token> GR_;
	std::unordered_map<char, Node> GR1_, GR2_, GR3_, GR4_;
	std::ifstream m_inputstream;
	double get_next_int(int radix = 10, bool after_dot = false)
	{
		char x = m_inputstream.peek();
		double result = (x - '0');
		int counter = 1;
		while (true)
		{
			m_inputstream.get();
			x = m_inputstream.peek();
			if (check_if_number(x))
			{
				result *= radix;
				result += (x - '0');
				++counter;
			}
			else
			{
				if (!after_dot)
					return result;
				else
					return result/pow(10, counter);
			}
		}
	}
	token read_after_dot()	//获取点号后面的小数（不包含点号），如果点号后面没有数字，则返回ERR_TOKEN
	{
		token result;
		char x_ = m_inputstream.peek();
		if (check_if_number(x_))
		{
			result.type = REAL_NUM;
			result.value.real_value = get_next_int(10, true);
			return result;
		}
		else
		{
			return ERR_TOKEN;
		}
	}
	token read_after_e()	//获取e后面的整数（不包含e），如果e后面没有数字，则返回ERR_TOKEN
	{
		token result;
		result.type = REAL_NUM;
		char x_ = m_inputstream.peek();
		if (x_ == '+')
		{
			m_inputstream.get();
			char x__ = m_inputstream.peek();
			if (check_if_number(x__))
				result.value.real_value = get_next_int();
			else
			{
				return ERR_TOKEN;
			}
		}
		else if (x_ == '-')
		{
			m_inputstream.get();
			char x__ = m_inputstream.peek();
			if (check_if_number(x__))
				result.value.real_value = - get_next_int();
			else
			{
				return ERR_TOKEN;
			}
		}
		else if (check_if_number(x_))
		{
			result.value.real_value = get_next_int();
		}
		else
		{
			return ERR_TOKEN;
		}
		return result;
	}
	token get_keyword()
	{
		std::string s;
		while (check_if_led(m_inputstream.peek()))
			s += m_inputstream.get();
		if (GR_.find(s) != GR_.end())
			return GR_[s];
		else if (s == "main")
		{
			token result_;
			result_.type = MAIN;
			return result_;
		}
		else
		{
			token result_ = create_str_token(VARNAME, s.c_str());
			return result_;
		}
	}
	token get_symbol()
	{
		char x = m_inputstream.get();
		if (x == '\"' || x == '\'')
		{
			std::string s = "\"";
			char x_ = m_inputstream.get();
			while (x_ != s[0] && x_ > 0)
			{
				s += x_;
				x_ = m_inputstream.get();
			}
			s += x;
			token result = create_str_token(STRING, s.c_str());
			return result;
		}
		else if (x == '/')
		{
			char x_ = m_inputstream.peek();
			if (x_ == '*')
			{
				m_inputstream.get();
				x_ = m_inputstream.peek();
				if (x_ < 0)
					return EOFLINE_TOKEN;
				else
					m_inputstream.get();
				char x__ = m_inputstream.peek();
				while ((x_ != '*' || x__ != '/') && x__ > 0)
				{
					x_ = x__;
					m_inputstream.get();
					x__ = m_inputstream.peek();
				}
				if (x__ > 0)
					m_inputstream.get();
				Symbol();
				return get_token();
			}
			else if (x_ == '/')
            {
                m_inputstream.get();
                while (m_inputstream.peek() != EOF && m_inputstream.peek() != '\n')
                {
                    m_inputstream.get();
                }
				Symbol();
				return get_token();
			}
		}
		else if (x == '.')
		{
			char x_ = m_inputstream.peek();
			if (check_if_number(x_))
			{
				token result2_ = read_after_dot();
				token result3_, result;
				x = m_inputstream.peek();
				if (x == 'e' || x == 'E')
				{
					m_inputstream.get();
					result3_ = read_after_e();
				}
				else
				{
					result3_.type = INT_NUM;
					result3_.value.real_value = 0;
				}
				if (result3_.type == ERR_TOKEN.type)
				{
					return ERR_TOKEN;
				}
				else
				{
					result.type = REAL_NUM;
					result.value.real_value = result2_.value.real_value * pow(10, result3_.value.real_value);
				}
				return result;
			}
		}
		if (GR1_.find(x) != GR1_.end())
		{
			return GR1_[x].result_;
		}
		else if (GR2_.find(x) != GR2_.end())
		{
			char x_ = m_inputstream.peek();
			if (x_ == '=')
			{
				m_inputstream.get();
				return GR2_[x].next->result_;
			}
			else
			{
				return GR2_[x].result_;
			}
		}
		else if (GR3_.find(x) != GR3_.end())
		{
			char x_ = m_inputstream.peek();
			if (x == x_)
			{
				m_inputstream.get();
				return GR3_[x].next->next->result_;
			}
			else if (x_ == '=')
			{
				m_inputstream.get();
				return GR3_[x].next->result_;
			}
			else
			{
				return GR3_[x].result_;
			}
		}
		else if (GR4_.find(x) != GR4_.end())
		{
			char x_ = m_inputstream.peek();
			if (x == x_)
			{
				m_inputstream.get();
				char x__ = m_inputstream.peek();
				if (x__ == '=')
				{
					m_inputstream.get();
					return GR4_[x].next->next->next->result_;
				}
				else
				{
					return GR4_[x].next->next->result_;
				}
			}
			else if (x_ == '=')
			{
				m_inputstream.get();
				return GR4_[x].next->result_;
			}
			else
			{
				return GR4_[x].result_;
			}
		}
		else
			return ERR_TOKEN;
	}
	token get_number()
	{
		token result;
		token result1_, result2_, result3_;
		char x = m_inputstream.peek();
		if (x == '0')
		{
			m_inputstream.get();
			x = m_inputstream.peek();
			if (x == 'x' || x == 'X')
			{
				result.type = INT_NUM;
				result.value.int_value = 0;
				while (true)
				{
					m_inputstream.get();
					x = m_inputstream.peek();
					if (check_if_number(x))
					{
						result.value.int_value *= 16;
						result.value.int_value += (x - '0');
					}
					else if (x >= 'a' && x <= 'f')
					{
						result.value.int_value *= 16;
						result.value.int_value += ((x - 'a') + 10);
					}
					else if (x >= 'A' && x <= 'F')
					{
						result.value.int_value *= 16;
						result.value.int_value += ((x - 'A') + 10);
					}
					else
					{
						return result;
					}
				}
			}
			else if (check_if_number(x))
			{
				result.type = INT_NUM;
				result.value.int_value = get_next_int(8);
				return result;
			}
			else
			{
				result1_.type = REAL_NUM;
				result1_.value.real_value = 0;
			}
		}
		else
		{
			result1_.type = REAL_NUM;
			result1_.value.real_value = get_next_int();
		}

		x = m_inputstream.peek();
		if (x == '.')
		{
			m_inputstream.get();
			result2_ = read_after_dot();
		}
		else
		{
			result2_.type = INT_NUM;
			result2_.value.real_value = 0;
		}

		x = m_inputstream.peek();
		if (x == 'e' || x == 'E')
		{
			m_inputstream.get();
			result3_ = read_after_e();
		}
		else
		{
			result3_.type = INT_NUM;
			result3_.value.real_value = 0;
		}

		if (result3_.type == ERR_TOKEN.type)
		{
			return ERR_TOKEN;
		}
		else if (result2_.type == ERR_TOKEN.type)
		{
			result.type = REAL_NUM;
			result.value.real_value = result1_.value.real_value * pow(10, result3_.value.real_value);
		}
		else if (result2_.type == REAL_NUM || result3_.type == REAL_NUM)
		{
			result.type = REAL_NUM;
			result.value.real_value = (result1_.value.real_value + result2_.value.real_value) * pow(10, result3_.value.real_value);
		}
		else
		{
			result.type = INT_NUM;
			result.value.int_value = (int)result1_.value.real_value;
		}
		return result;
	}

	void init()
	{
		for (auto i = 0; ID_[i][0]; ++i) GR_[ID_[i]] = NAME_[i];
		for (auto i = 0; ID1_[i]; ++i) GR1_[ID1_[i]] = NAME1_[i];
		for (auto i = 0; ID2_[i]; ++i) GR2_[ID2_[i]] = NAME2_[i];
		for (auto i = 0; ID3_[i]; ++i) GR3_[ID3_[i]] = NAME3_[i];
		for (auto i = 0; ID4_[i]; ++i) GR4_[ID4_[i]] = NAME4_[i];
		for (auto i = 0; ID_MOVE_[i][0]; ++i) GR_MOVE_[ID_MOVE_[i]] = NAME_MOVE_[i];
		for (auto i = 0; ID_STRANGE_[i][0]; ++i) GR_STRANGE_.insert(ID_STRANGE_[i]);
	}
public:
	Lex(const char * filename): m_inputstream(filename)
	{
		if (!m_inputstream){
			throw File_Error();
		}
		init();
	}
	token get_token()
	{
		while (check_if_blank(m_inputstream.peek())){
			m_inputstream.get();
			Symbol();
		}
		char peek_char = m_inputstream.peek();
		if (peek_char <= 0)
			return EOFLINE_TOKEN;
		else if (check_if_leu(peek_char))
			return Lex::get_keyword();
		else if (check_if_number(peek_char))
			return Lex::get_number();
		else
			return Lex::get_symbol();
	}
	word_seq get_seq_from_str(const std::string & str, int begin_index)
	{
		std::string temp_word;
		int pointer = begin_index;
		word_seq result;
		while (is_sep(str[pointer])) ++pointer;
		while (true) {
			if (is_sep(str[pointer]) || is_end(str[pointer])) {
				token temp_token;
				if (temp_word == var_str){
					temp_token.type = ABSTRACT_VAR;
				}
				else if (temp_word == num_str){
					temp_token.type = ABSTRACT_NUM;
				}
				else if (temp_word == spc_str){
                    temp_token = SPC_TOKEN;
				}
				else if (temp_word == eof_line_str){
                    temp_token = EOFLINE_TOKEN;
				}
				else if (GR_.find(temp_word) != GR_.end()){
					temp_token = GR_[temp_word];
				}
				else if (temp_word.size() == 1){
					if (GR1_.find(temp_word[0]) != GR1_.end()){
						temp_token = GR1_[temp_word[0]].result_;
					}
					else if (GR2_.find(temp_word[0]) != GR2_.end()){
						temp_token = GR2_[temp_word[0]].result_;
					}
					else if (GR3_.find(temp_word[0]) != GR3_.end()){
						temp_token = GR3_[temp_word[0]].result_;
					}
					else if (GR4_.find(temp_word[0]) != GR4_.end()){
						temp_token = GR4_[temp_word[0]].result_;
					}
				}
				else if (temp_word.size() == 2){
					if (GR2_.find(temp_word[0]) != GR2_.end()
						&& temp_word[1] == '='){
						temp_token = GR2_[temp_word[0]].next->result_;
					}
					else if (GR3_.find(temp_word[0]) != GR3_.end()
						&& temp_word[1] == '='){
						temp_token = GR3_[temp_word[0]].next->result_;
					}
					else if (GR3_.find(temp_word[0]) != GR3_.end()
						&& temp_word[0] == temp_word[1]){
						temp_token = GR3_[temp_word[0]].next->next->result_;
					}
					else if (GR4_.find(temp_word[0]) != GR4_.end()
						&& temp_word[1] == '='){
						temp_token = GR4_[temp_word[0]].next->result_;
					}
					else if (GR4_.find(temp_word[0]) != GR4_.end()
						&& temp_word[0] == temp_word[1]){
						temp_token = GR4_[temp_word[0]].next->next->result_;
					}
				}
				else if (temp_word.size() == 3){
					if (GR4_.find(temp_word[0]) != GR4_.end()
						&& temp_word[0] == temp_word[1]
						&& temp_word[2] == '='){
						temp_token = GR4_[temp_word[0]].next->next->next->result_;
					}
				}
				else{
                    temp_token = create_str_token(UNTERMINATOR, temp_word.c_str());
				}
				if (temp_word.size() > 0) result.push_back(temp_token);
				if (is_end(str[pointer])) break;
				++pointer;
				temp_word = "";
				while (is_sep(str[pointer])) ++pointer;
			}
			else temp_word += str[pointer++];
		}
		return result;
	}
	typedef struct {
		move_type move;
		token strange_token;
	} move_group;
	move_group get_move(const std::string & str) {
		int pointer = 0;
		move_group result;
		while (!is_move_begin(str[pointer])) pointer++;
		pointer++;
		while (is_sep(str[pointer])) pointer++;
		std::string temp_str;
		while (is_alpha(str[pointer]))	temp_str += str[pointer++];
		while (is_sep(str[pointer])) pointer++;
		if (GR_MOVE_.find(temp_str) != GR_MOVE_.end())
			result.move = GR_MOVE_[temp_str];
		temp_str = "";
		while (!is_end(str[pointer]) && !is_sep(str[pointer])) temp_str += str[pointer++];
		if (GR_STRANGE_.find(temp_str) != GR_STRANGE_.end())
			result.strange_token = create_str_token(STRANGE_TOKEN, temp_str.c_str());
		else
			result.strange_token = EOFLINE_TOKEN;
		return result;
	}
	grammar get_grammar()
	{
		static unsigned index = 1;
		std::string grammar_text;
		std::getline(m_inputstream, grammar_text);
		if (!m_inputstream)
			return EOF_GRAMMAR;
		else{
			grammar result;
			int pointer = 0;
			while (is_sep(grammar_text[pointer])) ++pointer;
			int before_arrow = grammar_text.find("->");
			if (before_arrow != std::string::npos){
				std::string temp_before_word;
				while (pointer != before_arrow && !is_sep(grammar_text[pointer])) temp_before_word += grammar_text[pointer++];
				while (pointer < before_arrow) ++pointer;
				pointer += sizeof("->") / sizeof(char);
                result.before_word = create_str_token(UNTERMINATOR, temp_before_word.c_str());
				result.after_words = get_seq_from_str(grammar_text, pointer);
				move_group temp_group = get_move(grammar_text);
				result.move = temp_group.move;
				result.strange_token = temp_group.strange_token;
				return result;
			}
		}
		++index;
	}
};
const std::string Lex::ID_[] = {
	"char",	"double", "float", "int", "short", "void",
	"case",	"break", "continue", "const", "do", "else",
	"for", "goto", "if", "return", "switch", "while",
	print_str, input_str, "main", ""
};
const token Lex::NAME_[] = {
	token{TYPENAME, Type_CHAR},
	token{TYPENAME, Type_DOUBLE},
	token{TYPENAME, Type_FLOAT},
	token{TYPENAME, Type_INT},
	token{TYPENAME, Type_SHORT},
	token{TYPENAME, Type_VOID},
	token{CONTROLLER, Controller_CASE},
	token{CONTROLLER, Controller_BREAK},
	token{CONTROLLER, Controller_CONTINUE},
	token{CONTROLLER, Controller_CONST},
	token{CONTROLLER, Controller_DO},
	token{CONTROLLER, Controller_ELSE},
	token{CONTROLLER, Controller_FOR},
	token{CONTROLLER, Controller_GOTO},
	token{CONTROLLER, Controller_IF},
	token{CONTROLLER, Controller_RETURN},
	token{CONTROLLER, Controller_SWITCH},
	token{CONTROLLER, Controller_WHILE},
	token{CONTROLLER, Controller_PRINT},
	token{CONTROLLER, Controller_INPUT},
	token{CONTROLLER, Controller_MAIN},
};
const char Lex::ID1_[] = "()[]~?:{},;";
const Lex::Node Lex::NAME1_[] = {
	{token{OPERATOR, Operator_LEFT_BRACKET}, nullptr},
	{token{OPERATOR, Operator_RIGHT_BRACKET},nullptr},
	{token{OPERATOR, Operator_LEFT_SQUARE_BRACKET}, nullptr},
	{token{OPERATOR, Operator_RIGHT_SQUARE_BRACKET}, nullptr},
	{token{OPERATOR, Operator_WAVY_LINE}, nullptr},
	{token{OPERATOR, Operator_QUESTION_MARK}, nullptr},
	{token{OPERATOR, Operator_COLON}, nullptr},
	{token{DELIMITER, Delimiter_LEFT_BRACE}, nullptr},
	{token{DELIMITER, Delimiter_RIGHT_BRACE}, nullptr},
	{token{DELIMITER, Delimiter_COMMA}, nullptr},
	{token{DELIMITER, Delimiter_SEMICOLON}, nullptr}
};
const char Lex::ID2_[] = "*/%!=^";
const Lex::Node Lex::NAME2_[] = {
	{token{OPERATOR, Operator_MULTIPLE},
		new Node({token{OPERATOR, Operator_MULTIPLE_ASSIGNMENT}, nullptr})},
	{token{OPERATOR, Operator_DIVISION},
		new Node({token{OPERATOR, Operator_DIVISION_ASSIGNMENT}, nullptr})},
	{token{OPERATOR, Operator_REMAINDER},
		new Node({token{OPERATOR, Operator_REMAINDER_ASSIGNMENT}, nullptr})},
	{token{OPERATOR, Operator_EXCLAMATION_MARK},
		new Node({token{OPERATOR, Operator_NOT_EQUAL}, nullptr})},
	{token{OPERATOR, Operator_ASSIGNMENT},
		new Node({token{OPERATOR, Operator_EQUAL}, nullptr})},
	{token{OPERATOR, Operator_POWER},
		new Node({token{OPERATOR, Operator_POWER_ASSIGNMENT}, nullptr})}
};
const char Lex::ID3_[] = "+-&|";
const Lex::Node Lex::NAME3_[] = {
	{token{OPERATOR, Operator_PLUS},
		new Node({token{OPERATOR, Operator_PLUS_ASSIGNMENT},
			new Node({token{OPERATOR, Operator_PLUS_PLUS}, nullptr})})},
	{token{OPERATOR, Operator_MINUS},
		new Node({token{OPERATOR, Operator_MINUS_ASSIGNMENT},
			new Node({token{OPERATOR, Operator_MINUS_MINUS}, nullptr})})},
	{token{OPERATOR, Operator_AND},
		new Node({token{OPERATOR, Operator_AND_ASSIGNMENT},
			new Node({token{OPERATOR, Operator_AND_AND}, nullptr})})},
	{token{OPERATOR, Operator_OR},
		new Node({token{OPERATOR, Operator_OR_ASSIGNMENT},
			new Node({token{OPERATOR, Operator_OR_OR}, nullptr})})},
};
const char Lex::ID4_[] = "<>";
const Lex::Node Lex::NAME4_[] = {
	{token{OPERATOR, Operator_GREATER},
		new Node({token{OPERATOR, Operator_GREATER_OR_EQUAL},
			new Node({token{OPERATOR, Operator_RIGHT_SHIFT},
				new Node({token{OPERATOR, Operator_RIGHT_SHIFT_ASSIGNMENT}, nullptr})})})},
	{token{OPERATOR, Operator_LESS},
		new Node({token{OPERATOR, Operator_LESS_OR_EQUAL},
			new Node({token{OPERATOR, Operator_LEFT_SHIFT},
				new Node({token{OPERATOR, Operator_LEFT_SHIFT_ASSIGNMENT}, nullptr})})})},
};

const std::string Lex::ID_MOVE_[] = {"makeleaf", "makenode", "same", "null", ""};
const move_type Lex::NAME_MOVE_[] = {makeleaf, makenode, same, null};
const std::string Lex::ID_STRANGE_[] = { "scanf", "printf", "if", "while", "do", "for", "=", "!",
"*", "/", "%", "+", "-", ">", "<", ">=", "<=", "==", "!=", "^", "&&", "||" , "var_declaration","conpound", ""};
#endif
