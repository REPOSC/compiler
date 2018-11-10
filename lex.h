#include "type.h"
#include "InputStream.h"
#include "judgement.h"
#include "base.h"
#include "debug.h"
#include <unordered_map>
#include <string>
#include <cstring>
#include <cmath>
#ifndef LEX__32
#define LEX__32

typedef struct {} File_Error;
#define Lex__DEBUG

#ifdef Lex__DEBUG
#define GetFromStream(x) _GetFromStream(x, debug_str)
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
			//if (debug_str == "\n") printf("读取到换行符，忽略\n");
			//else if (debug_str == "\r") printf("读取到回车符号，忽略\n");
			//else if (debug_str == " ") printf("读取到空格，忽略\n");
			//else if (debug_str == "\t") printf("读取到制表符，忽略\n");
			//else printf("【%s】，忽略\n", debug_str.c_str());
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
	const static std::string ID_[];
	const static token NAME_[];
	const static char ID1_[], ID2_[], ID3_[], ID4_[];
	const static Node NAME1_[], NAME2_[], NAME3_[], NAME4_[];
	
private:
	std::unordered_map<std::string, token> GR_;
	std::unordered_map<char, Node> GR1_, GR2_, GR3_, GR4_;
	InputStream m_inputstream;
	double get_next_int(int radix = 10, bool after_dot = false)
	{
		char x = PeekFromStream(&m_inputstream);
		double result = (x - '0');
		int counter = 1;
		while (true)
		{
			GetFromStream(&m_inputstream);
			x = PeekFromStream(&m_inputstream);
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
		char x_ = PeekFromStream(&m_inputstream);
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
		char x_ = PeekFromStream(&m_inputstream);
		if (x_ == '+')
		{
			GetFromStream(&m_inputstream);
			char x__ = PeekFromStream(&m_inputstream);
			if (check_if_number(x__))
				result.value.real_value = get_next_int();
			else
			{
				return ERR_TOKEN;
			}
		}
		else if (x_ == '-')
		{
			GetFromStream(&m_inputstream);
			char x__ = PeekFromStream(&m_inputstream);
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
		while (check_if_led(PeekFromStream(&m_inputstream)))
			s += GetFromStream(&m_inputstream);
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
			token result_;
			result_.type = VARNAME;
			result_.value.var_name = new char[strlen(s.c_str()) + 1];
			strcpy(result_.value.var_name, s.c_str());
			return result_;
		}
	}
	token get_symbol()
	{
		char x = GetFromStream(&m_inputstream);
		if (x == '\"' || x == '\'')
		{			
			std::string s = "\"";
			char x_ = GetFromStream(&m_inputstream);
			while (x_ != s[0] && x_ > 0)
			{
				s += x_;
				x_ = GetFromStream(&m_inputstream);
			}
			token result;
			s += x;
			result.type = STRING;
			result.value.str_name = new char[strlen(s.c_str()) + 1];
			strcpy(result.value.str_name, s.c_str());
			return result;		
		}
		else if (x == '/')
		{
			char x_ = PeekFromStream(&m_inputstream);
			if (x_ == '*')
			{
				GetFromStream(&m_inputstream);
				x_ = PeekFromStream(&m_inputstream);
				if (x_ < 0)
					return EOF_TOKEN;
				else
					GetFromStream(&m_inputstream);
				char x__ = PeekFromStream(&m_inputstream);
				while ((x_ != '*' || x__ != '/') && x__ > 0)
				{
					x_ = x__;
					GetFromStream(&m_inputstream);
					x__ = PeekFromStream(&m_inputstream);
				}
				if (x__ > 0)
					GetFromStream(&m_inputstream);
				Symbol();
				return get_token();
			}
		}
		else if (x == '.')
		{
			char x_ = PeekFromStream(&m_inputstream);
			if (check_if_number(x_))
			{
				token result2_ = read_after_dot();
				token result3_, result;
				x = PeekFromStream(&m_inputstream);
				if (x == 'e' || x == 'E')
				{
					GetFromStream(&m_inputstream);
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
			char x_ = PeekFromStream(&m_inputstream);
			if (x_ == '=')
			{
				GetFromStream(&m_inputstream);
				return GR2_[x].next->result_;
			}
			else
			{
				return GR2_[x].result_;
			}				
		}
		else if (GR3_.find(x) != GR3_.end())
		{
			char x_ = PeekFromStream(&m_inputstream);
			if (x == x_)
			{
				GetFromStream(&m_inputstream);
				return GR3_[x].next->next->result_;
			}
			else if (x_ == '=')
			{
				GetFromStream(&m_inputstream);
				return GR3_[x].next->result_;
			}
			else
			{
				return GR3_[x].result_;
			}				
		}
		else if (GR4_.find(x) != GR4_.end())
		{
			char x_ = PeekFromStream(&m_inputstream);
			if (x == x_)
			{
				GetFromStream(&m_inputstream);
				char x__ = PeekFromStream(&m_inputstream);
				if (x__ == '=')
				{
					GetFromStream(&m_inputstream);
					return GR4_[x].next->next->next->result_;
				}
				else
				{
					return GR4_[x].next->next->result_;
				}					
			}
			else if (x_ == '=')
			{
				GetFromStream(&m_inputstream);
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
		char x = PeekFromStream(&m_inputstream);
		if (x == '0')
		{
			GetFromStream(&m_inputstream);
			x = PeekFromStream(&m_inputstream);
			if (x == 'x' || x == 'X') 
			{
				result.type = INT_NUM;
				result.value.int_value = 0;
				while (true)
				{
					GetFromStream(&m_inputstream);
					x = PeekFromStream(&m_inputstream);
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

		x = PeekFromStream(&m_inputstream);
		if (x == '.')
		{
			GetFromStream(&m_inputstream);
			result2_ = read_after_dot();
		}
		else
		{
			result2_.type = INT_NUM;
			result2_.value.real_value = 0;
		}

		x = PeekFromStream(&m_inputstream);
		if (x == 'e' || x == 'E')
		{
			GetFromStream(&m_inputstream);
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
	}
public:
	Lex(const char * filename)
	{
		init();
		m_inputstream = CreateStream(filename);
		if (m_inputstream.file == NULL){
			throw File_Error();
		}
	}
	token get_token()
	{
		while (check_if_blank(PeekFromStream(&m_inputstream))){
			GetFromStream(&m_inputstream);
			Symbol();
		}
		char peek_char = PeekFromStream(&m_inputstream);
		if (peek_char <= 0)
			return EOF_TOKEN;
		else if (check_if_leu(peek_char))
			return Lex::get_keyword();
		else if (check_if_number(peek_char))
			return Lex::get_number();
		else
			return Lex::get_symbol();
	}
};
const std::string Lex::ID_[] = {
	"char",	"double", "float", "int", "short", "void",
	"case",	"break", "continue", "const", "do", "else", "for", "goto", "if", "return", "switch", "while",
	""
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
	token{CONTROLLER, Controller_WHILE}
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
const char Lex::ID2_[] = "*/%!=";
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
		new Node({token{OPERATOR, Operator_EQUAL}, nullptr})}
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
#endif
