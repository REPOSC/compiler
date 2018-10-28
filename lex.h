#include "type.h"
#include "InputStream.h"
#include "judgement.h"
#include "base.h"
#include <unordered_map>
#include <string>
#include <cstring>
#include <cmath>

#ifndef LEX__32
#define LEX__32

class Lex
{
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
	double get_next_int(int radix = 10)
	{
		char x = PeekFromStream(&m_inputstream);
		double result = (x - '0');
		while (true)
		{
			GetFromStream(&m_inputstream);
			x = PeekFromStream(&m_inputstream);
			if (check_if_number(x))
			{
				result *= radix;
				result += (x - '0');
			}
			else
			{
				return result;
			}
		}
	}
	double read_after_dot()
	{
		double result = 0;
		double radix = 1;
		char x = PeekFromStream(&m_inputstream);
        while (check_if_number(x))
        {
			GetFromStream(&m_inputstream);
			radix *= 10;
			result += (x - '0')/radix;
			x = PeekFromStream(&m_inputstream);
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
		if (x == '"')
		{
			std::string s = "\"";
			char x_ = GetFromStream(&m_inputstream);
			while (x_ != '"')
			{
				s += x_;
				x_ = GetFromStream(&m_inputstream);
			}
			token result;
			s = s + "\"";
			result.type = STRING;
			result.value.var_name = new char[strlen(s.c_str()) + 1];
			strcpy(result.value.var_name, s.c_str());
			return result;		
		}
		else if (x == '/')
		{
			char x_ = PeekFromStream(&m_inputstream);
			if (x_ == '*')
			{
				GetFromStream(&m_inputstream);
				x_ = GetFromStream(&m_inputstream);
				char x__ = GetFromStream(&m_inputstream);
				while ((x_ != '*' || x__ != '/') && x__ > 0)
				{
					x_ = x__;
					x__ = GetFromStream(&m_inputstream);
				}
				return get_token();
			}
		}
		else if (x == '.')
		{
			if (check_if_number(PeekFromStream(&m_inputstream)))
			{
				token result_;
				result_.type = REAL_NUM;
				result_.value.real_value = read_after_dot();
				x = PeekFromStream(&m_inputstream);
				if (x == 'e' || x == 'E')
				{
					GetFromStream(&m_inputstream);
					x = PeekFromStream(&m_inputstream);
					if (x == '+')
					{
						GetFromStream(&m_inputstream);
						result_.value.real_value *= pow(10, get_next_int());
					}
					else if (x == '-')
					{
						GetFromStream(&m_inputstream);
						result_.value.real_value *= pow(10, get_next_int() * (-1));
					}
					else
					{
						result_.value.real_value *= pow(10, get_next_int());
					}
				}
				return result_;
			}
		}
		if (GR1_.find(x) != GR1_.end())
			return GR1_[x].result_;
		else if (GR2_.find(x) != GR2_.end())
		{
			char x_ = PeekFromStream(&m_inputstream);
			if (x_ == '=')
			{
				GetFromStream(&m_inputstream);
				return GR2_[x].next->result_;
			}
			else
				return GR2_[x].result_;
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
				return GR3_[x].result_;
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
					return GR4_[x].next->next->result_;
			}
			else if (x_ == '=')
			{
				GetFromStream(&m_inputstream);
				return GR4_[x].next->result_;
			}
			else
				return GR4_[x].result_;
		}
		else
			return ERR_TOKEN;
	}
	token get_number()
	{
		token result_;
		char x = PeekFromStream(&m_inputstream);
		if (x == '0')
		{
            GetFromStream(&m_inputstream);
			x = PeekFromStream(&m_inputstream);
			if (x == 'x' || x == 'X') 
			{
				result_.type = INT_NUM;
				result_.value.int_value = 0;
				while (true)
				{
					GetFromStream(&m_inputstream);
					x = PeekFromStream(&m_inputstream);
					if (check_if_number(x))
					{
						result_.value.int_value *= 16;
						result_.value.int_value += (x - '0');
					}
					else if (x >= 'a' && x <= 'f')
					{
						result_.value.int_value *= 16;
						result_.value.int_value += ((x - 'a') + 10);
					}
					else if (x >= 'A' && x <= 'F')
					{
						result_.value.int_value *= 16;
						result_.value.int_value += ((x - 'A') + 10);
					}
					else
						return result_;
				}
			}
			else if (x == '.')
			{
				GetFromStream(&m_inputstream);
				result_.type = REAL_NUM;
				result_.value.real_value = read_after_dot();
				x = PeekFromStream(&m_inputstream);
				if (x == 'e' || x == 'E')
				{
					GetFromStream(&m_inputstream);
					x = PeekFromStream(&m_inputstream);
					if (x == '+')
					{
						GetFromStream(&m_inputstream);
						result_.value.real_value *= pow(10, get_next_int());
					}
					else if (x == '-')
					{
						GetFromStream(&m_inputstream);
						result_.value.real_value *= pow(10, get_next_int() * (-1));
					}
					else
					{
						result_.value.real_value *= pow(10, get_next_int());
					}
				}
				return result_;
			}
			else if (!check_if_number(x)) 
			{
				result_.type = INT_NUM;
				result_.value.int_value = 0;
				return result_;
			}
			else
			{
				result_.type = INT_NUM;
				result_.value.int_value = get_next_int(8);
				return result_;
			}
		}
		else
		{
			result_.type = REAL_NUM;
			result_.value.real_value = get_next_int();
			x = PeekFromStream(&m_inputstream);
			if (x == '.')
			{
				GetFromStream(&m_inputstream);
				result_.value.real_value += read_after_dot();
				x = PeekFromStream(&m_inputstream);
				if (x == 'e' || x == 'E')
				{
					GetFromStream(&m_inputstream);
					x = PeekFromStream(&m_inputstream);
					if (x == '+')
					{
						GetFromStream(&m_inputstream);
						result_.value.real_value *= pow(10, get_next_int());
					}
					else if (x == '-')
					{
						GetFromStream(&m_inputstream);
						result_.value.real_value *= pow(10, get_next_int() * (-1));
					}
					else
					{
						result_.value.real_value *= pow(10, get_next_int());
					}
				}
			}
			else if (x == 'e' || x == 'E')
			{
				GetFromStream(&m_inputstream);
				x = PeekFromStream(&m_inputstream);
				if (x == '+')
				{
					GetFromStream(&m_inputstream);
					result_.value.real_value *= pow(10, get_next_int());
				}
				else if (x == '-')
				{
					GetFromStream(&m_inputstream);
					result_.value.real_value *= pow(10, get_next_int() * (-1));
				}
				else
				{
					result_.value.real_value *= pow(10, get_next_int());
				}
			}
			else
			{
				result_.type = INT_NUM;
				result_.value.int_value = (int)result_.value.real_value;				
			}
			return result_;
		}
	}

	InputStream m_inputstream;
	void init()
	{
		for (auto i = 0; ID_[i] != ""; ++i) GR_[ID_[i]] = NAME_[i];
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
	}
	token get_token()
	{
		while (check_if_blank(PeekFromStream(&m_inputstream)))
			GetFromStream(&m_inputstream);
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
