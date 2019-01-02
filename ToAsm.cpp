#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <vector>
#include "judgement.h"
#include "type.h"
#include <stdlib.h>
#include <string>
#include <cstring>
#include <Windows.h>
#define ERROR -1

typedef std::vector<std::string> instruction;
typedef std::string three_items[3];

class ToAsm{
	static std::map<std::string, unsigned> symbol_table;
	static std::vector<std::pair<std::string, std::string> > temp_symbol;
	static std::vector<std::string> user_symbol;
	static std::vector<instruction> result_codes;
	static std::string float_zero;
	static std::string int_input_fmt_addr, int_output_fmt_addr;
	static std::string real_input_fmt_addr, real_output_fmt_addr;
	inline static std::wstring StringToWString(const std::string &str) {
		int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		wchar_t *wide = new wchar_t[num];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
		std::wstring w_str(wide);
		delete[] wide;
		return w_str;
	}
	inline static std::string var(const std::string &varname){
		return "var" + varname;
	}
	inline static std::string label(const std::string &labelname){
		return "label" + labelname;
	}
	inline static std::string temp(const std::string &varname){
		return "temp" + varname;
	}
	inline static unsigned judge(const std::string &tokenname){
		if (check_if_leu(tokenname[0])){
			return ABSTRACT_VAR;
		}
		for (auto it : tokenname){
			if (it == '.')
				return REAL_NUM;
			else if (!(check_if_number(it) || it == '.'))
				throw ERROR;
		}
		return INT_NUM;
	}
	static std::string rand_register(){
		std::string registers[] = {
			"eax",
			"ebx",
			"ecx",
			"edx"
		};
		int setect = rand() % (sizeof(registers) / sizeof(std::string));
		return registers[setect];
	}
	static std::string _tmp_insert(std::string _value){
		std::string temp_name = temp(std::to_string(temp_symbol.size()));
		if (judge(_value) == INT_NUM){
			temp_symbol.push_back({ temp_name, _value + "." });
		}
		else if (judge(_value) == REAL_NUM){
			temp_symbol.push_back({ temp_name, _value });
		}
		symbol_table[temp_name] = REAL_NUM;
		return temp_name;
	}
	static bool judge_and_change_to_real(three_items ti, unsigned judge_result[]){
		bool is_real = false;
		for (int i = 0; i < 2; ++i){
			judge_result[i] = judge(ti[i]);
			if (judge_result[i] == REAL_NUM){
				is_real = true;
			}
			else if (judge_result[i] == ABSTRACT_VAR){
				ti[i] = var(ti[i]);
				if (symbol_table.find(ti[i]) == symbol_table.end()){
					throw ERROR;
				}
				if (symbol_table[ti[i]] == REAL_NUM){
					is_real = true;
				}
			}
		}
		if (judge(ti[2]) != ABSTRACT_VAR){
			throw ERROR;
		}
		ti[2] = var(ti[2]);
		if (symbol_table.find(ti[2]) == symbol_table.end()){
			throw ERROR;
		}
		return is_real;
	}
	static instruction create_int(three_items ti){
		//(i, var, _, _)
		ti[0] = var(ti[0]);
		if (symbol_table.find(ti[0]) != symbol_table.end()){
			throw ERROR;
		}
		symbol_table[ti[0]] = INT_NUM;
		user_symbol.push_back(ti[0]);
		return instruction();
	}
	static instruction create_double(three_items ti){
		//(d, var, _, _)
		ti[0] = var(ti[0]);
		if (symbol_table.find(ti[0]) != symbol_table.end()){
			throw ERROR;
		}
		symbol_table[ti[0]] = REAL_NUM;
		user_symbol.push_back(ti[0]);
		return instruction();
	}
	static instruction jmp(three_items ti){
		//(j, _, _, label)
		return{ "jmp " + label(ti[2]) };
	}
	static instruction jmp_equal(three_items ti){
		//(jz, var/i/d, _, label)
		unsigned judge_result = judge(ti[0]);
		instruction result;
		if (judge_result == ABSTRACT_VAR) {
			ti[0] = var(ti[0]);
			if (symbol_table.find(ti[0]) == symbol_table.end()){
				throw ERROR;
			}
			if (symbol_table[ti[0]] == INT_NUM){
				std::string s[] = {
					"cmp " + ti[0] + ",0",
					"jz " + label(ti[2])
				};
				for (int i = 0;
					i < sizeof(s) / sizeof(std::string);
					++i){
					result.push_back(s[i]);
				}
			}
			else if (symbol_table[ti[0]] == REAL_NUM){
				std::string s[] = {
					"fld " + float_zero,
					"fld " + ti[0],
					"fcomip st,st(1)",
					"fstp " + float_zero,
					"jz " + label(ti[2])
				};
				for (int i = 0;
					i < sizeof(s) / sizeof(std::string);
					++i){
					result.push_back(s[i]);
				}
			}
		}
		else if (judge_result == INT_NUM){
			std::string s[] = {
				"cmp " + ti[0] + ",0",
				"jz " + label(ti[2])
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else if (judge_result == REAL_NUM){
			std::string tmp_var = _tmp_insert(ti[0]);
			std::string s[] = {
				"fld " + float_zero,
				"fld " + tmp_var,
				"fcomip st st(1)",
				"fstp " + float_zero,
				"jz " + label(ti[2])
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		return result;
	}
	static instruction jmp_not_equal(three_items ti){
		//(jnz, var/i/d, _, label)
		unsigned judge_result = judge(ti[0]);
		instruction result;
		if (judge_result == ABSTRACT_VAR) {
			ti[0] = var(ti[0]);
			if (symbol_table.find(ti[0]) == symbol_table.end()){
				throw ERROR;
			}
			if (symbol_table[ti[0]] == INT_NUM){
				std::string s[] = {
					"cmp " + ti[0] + ",0",
					"jnz " + label(ti[2])
				};
				for (int i = 0;
					i < sizeof(s) / sizeof(std::string);
					++i){
					result.push_back(s[i]);
				}
			}
			else if (symbol_table[ti[0]] == REAL_NUM){
				std::string s[] = {
					"fld " + float_zero,
					"fld " + ti[0],
					"fcomip st st(1)",
					"fstp " + float_zero,
					"jnz " + label(ti[2])
				};
				for (int i = 0;
					i < sizeof(s) / sizeof(std::string);
					++i){
					result.push_back(s[i]);
				}
			}
		}
		else if (judge_result == INT_NUM){
			std::string s[] = {
				"cmp " + ti[0] + ",0",
				"jnz " + label(ti[2])
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else if (judge_result == REAL_NUM){
			std::string tmp_var = _tmp_insert(ti[0]);
			std::string s[] = {
				"fld " + float_zero,
				"fld " + tmp_var,
				"fcomip st st(1)",
				"fstp " + float_zero,
				"jnz " + label(ti[2])
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		return result;
	}
	static instruction mov(three_items ti){
		//(=, var/i/d, _, var)
		unsigned judge_result[] = { judge(ti[0]), judge(ti[2]) };
		if (judge_result[0] == ABSTRACT_VAR){
			ti[0] = var(ti[0]);
			if (symbol_table.find(ti[0]) == symbol_table.end()){
				throw ERROR;
			}
		}
		if (judge_result[1] != ABSTRACT_VAR){
			throw ERROR;
		}
		else {
			ti[2] = var(ti[2]);
			if (symbol_table.find(ti[2]) == symbol_table.end()){
				throw ERROR;
			}
			if (symbol_table[ti[2]] == REAL_NUM){
				if (judge_result[0] == REAL_NUM || judge_result[0] == INT_NUM){
					std::string temp_name = _tmp_insert(ti[0]);
					instruction result = {
						"fld " + temp_name,
						"fstp " + ti[2]
					};
					return result;
				}
				else if (judge_result[0] == ABSTRACT_VAR){
					if (symbol_table[ti[0]] == INT_NUM){
						instruction result = {
							"fild " + ti[0],
							"fstp " + ti[2]
						};
						return result;
					}
					else if (symbol_table[ti[0]] == REAL_NUM){
						instruction result = {
							"fld " + ti[0],
							"fstp " + ti[2]
						};
						return result;
					}
				}
			}
			else if (symbol_table[ti[2]] == INT_NUM){
				if (judge_result[0] == INT_NUM){
					instruction result = {
						"mov " + ti[2] + "," + ti[0],
					};
					return result;
				}
				else if (judge_result[0] == REAL_NUM){
					std::string temp_name = _tmp_insert(ti[0]);
					instruction result = {
						"fld " + temp_name,
						"fistp " + ti[2]
					};
					return result;
				}
				else if (judge_result[0] == ABSTRACT_VAR){
					if (symbol_table[ti[0]] == INT_NUM){
						std::string now_register = rand_register();
						instruction result = {
							"mov " + now_register + "," + ti[0],
							"mov " + ti[2] + "," + now_register
						};
						return result;
					}
					else if (symbol_table[ti[0]] == REAL_NUM){
						instruction result = {
							"fld " + ti[0],
							"fistp " + ti[2]
						};
						return result;
					}
				}
			}
		}
	}
	static instruction plus(three_items ti){
		//(+, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fadd " + ti[1],
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == INT_NUM){
				result.push_back("fistp " + ti[2]);
			}
			else if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fstp " + ti[2]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"mov " + now_register + "," + ti[0],
					"add " + now_register + "," + ti[1],
					"mov dword ptr[" + ti[2] + "]," + now_register
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fild dword ptr[" + ti[2] + "]");
				result.push_back("fstp " + ti[2]);
			}
		}
		return result;
	}
	static instruction minus(three_items ti){
		//(-, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fsub " + ti[1],
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == INT_NUM){
				result.push_back("fistp " + ti[2]);
			}
			else if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fstp " + ti[2]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"mov " + now_register + "," + ti[0],
					"sub " + now_register + "," + ti[1],
					"mov dword ptr[" + ti[2] + "]," + now_register
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fild dword ptr[" + ti[2] + "]");
				result.push_back("fstp " + ti[2]);
			}
		}
		return result;
	}
	static instruction multiple(three_items ti){
		//(*, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fmul " + ti[1],
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == INT_NUM){
				result.push_back("fistp " + ti[2]);
			}
			else if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fstp " + ti[2]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"mov " + now_register + "," + ti[0],
					"imul " + now_register + "," + ti[1],
					"mov dword ptr[" + ti[2] + "]," + now_register
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fild dword ptr[" + ti[2] + "]");
				result.push_back("fstp " + ti[2]);
			}
			else {
				result.push_back("mov " + ti[2] + "," + now_register);
			}
		}
		return result;
	}
	static instruction divide(three_items ti){
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fdiv " + ti[1],
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == INT_NUM){
				result.push_back("fistp " + ti[2]);
			}
			else if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fstp " + ti[2]);
			}
		}
		else {
			std::string s[] {
				"mov edx,0",
					"mov eax," + ti[0],
					"mov ebx," + ti[1],
					"idiv ebx",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
			if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fild dword ptr[" + ti[2] + "]");
				result.push_back("fstp " + ti[2]);
			}
		}
		return result;
	}
	static instruction mod(three_items ti){
		//(%, var/i, var/i, var)
		unsigned judge_result[2];
		instruction result;
		for (int i = 0; i < 2; ++i){
			judge_result[i] = judge(ti[i]);
			if (judge_result[i] == REAL_NUM){
				throw ERROR;
			}
			else if (judge_result[i] == ABSTRACT_VAR){
				ti[i] = var(ti[i]);
				if (symbol_table.find(ti[i]) == symbol_table.end()){
					throw ERROR;
				}
				if (symbol_table[ti[i]] == REAL_NUM){
					throw ERROR;
				}
			}
		}
		if (judge(ti[2]) != ABSTRACT_VAR){
			throw ERROR;
		}
		ti[2] = var(ti[2]);
		if (symbol_table.find(ti[2]) == symbol_table.end()){
			throw ERROR;
		}
		std::string s[] {
			"mov edx,0",
				"mov eax," + ti[0],
				"mov ebx," + ti[1],
				"idiv ebx",
				"mov dword ptr[" + ti[2] + "],edx"
		};
		for (int i = 0;
			i < sizeof(s) / sizeof(std::string);
			++i){
			result.push_back(s[i]);
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction above(three_items ti){
		//(>, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fld " + ti[1],
				"fcompp",
				"fnstsw ax",
				"test ah, 1h",
				"mov eax, 0",
				"setne al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
				"mov " + now_register + "," + ti[0],
				"cmp " + now_register + "," + ti[1],
				"setg al",
				"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction below(three_items ti){
		//(<, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fld " + ti[1],
				"fcompp",
				"fnstsw ax",
				"test ah, 41h",
				"mov eax, 0",
				"sete al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
				"mov " + now_register + "," + ti[0],
				"cmp " + now_register + "," + ti[1],
				"setl al",
				"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction above_equal(three_items ti){
		//(>=, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fld " + ti[1],
				"fcompp",
				"fnstsw ax",
				"test ah, 41h",
				"mov eax, 0",
				"setne al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
					"mov " + now_register + "," + ti[0],
					"cmp " + now_register + "," + ti[1],
					"setge al",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction below_equal(three_items ti){
		//(<=, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fld " + ti[1],
				"fcompp",
				"fnstsw ax",
				"test ah, 1",
				"mov eax, 0",
				"sete al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
					"mov " + now_register + "," + ti[0],
					"cmp " + now_register + "," + ti[1],
					"setle al",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction equal(three_items ti){
		//(==, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fld " + ti[1],
				"fcomip st,st(1)",
				"mov eax, 0",
				"sete al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
					"mov " + now_register + "," + ti[0],
					"cmp " + now_register + "," + ti[1],
					"sete al",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction not_equal(three_items ti){
		//(!=, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + ti[0],
				"fld " + ti[1],
				"fcomip st,st(1)",
				"mov eax, 0",
				"setne al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
					"mov " + now_register + "," + ti[0],
					"cmp " + now_register + "," + ti[1],
					"setne al",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction and(three_items ti){
		//(&&, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + float_zero,
				"fld " + ti[0],
				"fcomip st,st(1)",
				"setne al",
				"fld " + ti[1],
				"fcomip st,st(1)",
				"setne ah",
				"fstp " + float_zero,
				"test ah, al",
				"mov eax, 0",
				"setne al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
					"cmp " + ti[0] + ", 0",
					"setne al",
					"cmp " + ti[1] + ", 0",
					"setne ah",
					"test al, ah",
					"mov eax, 0",
					"setne al",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction or(three_items ti){
		//(||, var/i/d, var/i/d, var)
		instruction result;
		unsigned judge_result[2];
		bool is_real = judge_and_change_to_real(ti, judge_result);
		if (is_real){
			for (int i = 0; i < 2; ++i){
				if (judge_result[i] == REAL_NUM || judge_result[i] == INT_NUM){
					ti[i] = _tmp_insert(ti[i]);
				}
				else if (judge_result[i] == ABSTRACT_VAR && symbol_table[ti[i]] == INT_NUM){
					std::string temp_name = _tmp_insert("0");
					std::string _prev_mov[] = {
						"fild " + ti[i],
						"fstp " + temp_name
					};
					for (int i = 0;
						i < sizeof(_prev_mov) / sizeof(std::string);
						++i){
						result.push_back(_prev_mov[i]);
					}
					ti[i] = temp_name;
				}
			}
			std::string s[] = {
				"fld " + float_zero,
				"fld " + ti[0],
				"fcomip st,st(1)",
				"sete al",
				"fld " + ti[1],
				"fcomip st,st(1)",
				"sete ah",
				"fstp " + float_zero,
				"test ah, al",
				"mov eax, 1",
				"sete al",
				"mov dword ptr[" + ti[2] + "], eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		else {
			std::string now_register = rand_register();
			std::string s[] {
				"xor eax, eax",
					"cmp " + ti[0] + ", 0",
					"sete al",
					"cmp " + ti[1] + ", 0",
					"sete ah",
					"test al, ah",
					"mov eax, 0",
					"sete al",
					"mov dword ptr[" + ti[2] + "],eax"
			};
			for (int i = 0;
				i < sizeof(s) / sizeof(std::string);
				++i){
				result.push_back(s[i]);
			}
		}
		if (symbol_table[ti[2]] == REAL_NUM){
			result.push_back("fild dword ptr[" + ti[2] + "]");
			result.push_back("fstp " + ti[2]);
		}
		return result;
	}
	static instruction not(three_items ti){
		//(!, var/i/d, _, var)
		unsigned judge_result[] = { judge(ti[0]), judge(ti[2]) };
		instruction result;
		if (judge_result[0] == ABSTRACT_VAR){
			ti[0] = var(ti[0]);
			if (symbol_table.find(ti[0]) == symbol_table.end()){
				throw ERROR;
			}
		}
		if (judge_result[1] != ABSTRACT_VAR){
			throw ERROR;
		}
		else {
			ti[2] = var(ti[2]);
			if (symbol_table.find(ti[2]) == symbol_table.end()){
				throw ERROR;
			}
			if (judge_result[0] == INT_NUM ||
				judge_result[0] == ABSTRACT_VAR && symbol_table[ti[0]] == INT_NUM){
				std::string s[] = {
					"xor eax, eax",
					"cmp " + ti[0] + ",0",
					"sete al",
					"mov " + ti[2] + ",eax"
				};
				for (int i = 0;
					i < sizeof(s) / sizeof(std::string);
					++i){
					result.push_back(s[i]);
				}
			}
			else if (judge_result[0] == REAL_NUM){
				ti[0] = _tmp_insert(ti[0]);
			}

			if (judge_result[0] == ABSTRACT_VAR && symbol_table[ti[0]] == REAL_NUM){
				std::string s[] = {
					"xor eax, eax",
					"fld " + float_zero,
					"fld " + ti[0],
					"fcomip st,st(1)",
					"fstp " + float_zero,
					"sete al",
					"mov " + ti[2] + ",eax"
				};
				for (int i = 0;
					i < sizeof(s) / sizeof(std::string);
					++i){
					result.push_back(s[i]);
				}
			}
			if (symbol_table[ti[2]] == REAL_NUM){
				result.push_back("fild dword ptr[" + ti[2] + "]");
				result.push_back("fstp " + ti[2]);
			}
		}

		return result;
	}
	static instruction print(three_items ti){
		//(p, var/i/d, _, _)
		unsigned judge_result = judge(ti[0]);
		instruction result;
		if (judge_result == ABSTRACT_VAR){
			ti[0] = var(ti[0]);
			if (symbol_table.find(ti[0]) == symbol_table.end()){
				throw ERROR;
			}
		}
		if (judge_result == INT_NUM || judge_result == ABSTRACT_VAR && symbol_table[ti[0]] == INT_NUM){
			result.push_back("invoke crt_printf,addr " + int_output_fmt_addr + "," + ti[0]);
		}
		else if (judge_result == REAL_NUM || judge_result == ABSTRACT_VAR && symbol_table[ti[0]] == REAL_NUM){
			result.push_back("invoke crt_printf,addr " + real_output_fmt_addr + "," + ti[0]);
		}
		return result;
	}
	static instruction scan(three_items ti){
		//(s, var, _, _)
		unsigned judge_result = judge(ti[0]);
		instruction result;
		if (judge_result == ABSTRACT_VAR){
			ti[0] = var(ti[0]);
			if (symbol_table.find(ti[0]) == symbol_table.end()){
				throw ERROR;
			}
		}
		else{
			throw ERROR;
		}
		if (symbol_table[ti[0]] == INT_NUM){
			result.push_back("invoke crt_scanf,addr " + int_input_fmt_addr + ",addr " + ti[0]);
		}
		else if (judge_result == REAL_NUM || judge_result == ABSTRACT_VAR && symbol_table[ti[0]] == REAL_NUM){
			result.push_back("invoke crt_scanf,addr " + real_input_fmt_addr + ",addr " + ti[0]);
		}
		return result;
	}
	static instruction end(three_items ti){
		//(_, _, _, _)
		return{ "invoke ExitProcess, 0" };
	}

	static std::map<std::string, instruction(*) (three_items)> ins_table;
	static std::vector<std::string> opcs;
	static std::vector<instruction(*) (three_items)> opvs;
	static std::string masm32_directory;

	static void init(){
		for (int i = 0; i < opcs.size(); ++i){
			ins_table[opcs[i]] = opvs[i];
		}
		temp_symbol.push_back({ float_zero, "0." });
		symbol_table[float_zero] = REAL_NUM;
	}
	static void install_fmt(std::ofstream & resultfile) {
		resultfile << int_input_fmt_addr << " db '%d', 0" << std::endl;
		resultfile << real_input_fmt_addr << " db '%f', 0" << std::endl;
		resultfile << int_output_fmt_addr << " db '%d', 10, 0" << std::endl;
		resultfile << real_output_fmt_addr << " db '%f', 10, 0" << std::endl;
	}
	static void print_inits(std::ofstream & resultfile) {
		resultfile << ".686" << std::endl;
		resultfile << ".model flat, stdcall" << std::endl;
		resultfile << "include " + masm32_directory + "\\include\\msvcrt.inc" << std::endl;
		resultfile << "include " + masm32_directory + "\\include\\kernel32.inc" << std::endl;
		resultfile << "includelib " + masm32_directory + "\\lib\\msvcrt.lib" << std::endl;
		resultfile << "includelib " + masm32_directory + "\\lib\\kernel32.lib" << std::endl;
	}
	static void print_definations(std::ofstream & resultfile){
		for (auto it : temp_symbol){
			resultfile << it.first << " real8 " << it.second << std::endl;
		}
		for (auto it : user_symbol){
			resultfile << it;
			if (symbol_table[it] == INT_NUM)
				resultfile << " dd ?" << std::endl;
			else if (symbol_table[it] == REAL_NUM)
				resultfile << " real8 ?" << std::endl;
		}
	}
	static void print_init_fpu(std::ofstream & resultfile){
		resultfile << "finit" << std::endl;
	}
	static void print_instructions(std::ofstream & resultfile){
		resultfile << "start:" << std::endl;
		print_init_fpu(resultfile);
		for (int i = 0; i < result_codes.size(); ++i){
			resultfile << label(std::to_string(i + 1)) << ": " << std::endl;
			for (auto it : result_codes[i]){
				resultfile << it << std::endl;
			}
		}
		resultfile << "end start" << std::endl;
	}
public:
	static void set_masm32_file(){
		std::cout << "Please input the masm32 install directory (eg: C:\\masm32) " << std::endl;
		std::cin >> masm32_directory;
	}
	static void dispatch_file(std::ifstream & inputfile, std::ofstream & resultfile){
		std::string line_input;
		init();
		while (true){
			getline(inputfile, line_input);
			if (!inputfile)
				break;
			std::string l[4];
			int pointer = 0;
			while (line_input[pointer] != '(') ++pointer;
			++pointer;

			char terminals[] = { ',', ',', ',', ')' };
			for (int i = 0; i < 4; ++i){
				while (is_sep(line_input[pointer])) ++pointer;
				while (line_input[pointer] != terminals[i]) l[i] += line_input[pointer++];
				++pointer;
			}
			three_items it = { l[1], l[2], l[3] };
			if (ins_table.find(l[0]) == ins_table.end()){
				throw ERROR;
			}
			instruction ins = ins_table[l[0]](it);
			result_codes.push_back(ins);
		}
		print_inits(resultfile);
		resultfile << ".data" << std::endl;
		install_fmt(resultfile);
		print_definations(resultfile);
		resultfile << ".code" << std::endl;
		print_instructions(resultfile);
	}
	static void call_masm(std::wstring filename){
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = true;
		PROCESS_INFORMATION pi;
		std::wstring Wmasm32_directory = StringToWString(masm32_directory + "\\bin");
		std::wstring full_path = Wmasm32_directory + L"\\ml.exe /c /coff " + filename + L".asm";
		wchar_t * new_full_path = new wchar_t[256];
		lstrcpyW(new_full_path, full_path.c_str());
		bool create_result = CreateProcess(NULL,
			new_full_path,
			NULL,
			NULL,
			false,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&si,
			&pi);
		if (!create_result){
			std::cerr << "Cannot start assembler masm32, please check!" << std::endl;
			return;
		}
		DWORD wait_result = WaitForSingleObject(pi.hProcess, INFINITE);
		if (wait_result != WAIT_OBJECT_0){
			std::cerr << "Masm32 run failed, please check!" << std::endl;
		}
		full_path = Wmasm32_directory + L"\\link.exe /subsystem:console " + filename + L".obj";
		lstrcpyW(new_full_path, full_path.c_str());
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = true;
		create_result = CreateProcess(NULL,
			new_full_path,
			NULL,
			NULL,
			false,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&si,
			&pi);
		if (!create_result){
			std::cerr << "Cannot start linker, please check!" << std::endl;
			return;
		}
		wait_result = WaitForSingleObject(pi.hProcess, INFINITE);
		if (wait_result != WAIT_OBJECT_0){
			std::cerr << "Linker run failed, please check!" << std::endl;
		}
		wchar_t now_directory[256];
		GetCurrentDirectory(256, now_directory);
		full_path = std::wstring(now_directory) + L"\\" + filename + L".exe";
		lstrcpyW(new_full_path, full_path.c_str());
		WIN32_FIND_DATA find_data;
		HANDLE hwnd = FindFirstFile(new_full_path, &find_data);
		if (hwnd != INVALID_HANDLE_VALUE){
			std::cout << "Generate success!" << std::endl;
		}
		else {
			std::cout << "Generate exe file failed, please check!" << std::endl;
		}
		delete []new_full_path;
	}
};
std::vector<std::string> ToAsm::opcs = {
	"i", "d", "j", "jz", "jnz", "=", "+", "-", "*", "/", "%", ">",
	"<", ">=", "<=", "==", "!=", "&&", "||", "!", "p", "s", "_"
};
std::vector<instruction(*) (three_items)> ToAsm::opvs = {
	create_int, create_double, jmp, jmp_equal, jmp_not_equal,
	mov, plus, minus, multiple, divide, mod, above, below,
	above_equal, below_equal, equal, not_equal, and, or,
	not, print, scan, end
};
std::string ToAsm::float_zero = "float_zero";
std::string ToAsm::int_input_fmt_addr = "int_input_fmt_addr";
std::string ToAsm::real_input_fmt_addr = "real_input_fmt_addr";
std::string ToAsm::int_output_fmt_addr = "int_output_fmt_addr";
std::string ToAsm::real_output_fmt_addr = "real_output_fmt_addr";
std::map<std::string, unsigned> ToAsm::symbol_table;
std::vector<std::pair<std::string, std::string> > ToAsm::temp_symbol;
std::vector<std::string> ToAsm::user_symbol;
std::vector<instruction> ToAsm::result_codes;
std::map<std::string, instruction(*) (three_items)> ToAsm::ins_table;
std::string ToAsm::masm32_directory;

int main(){	
	ToAsm::set_masm32_file();
	ToAsm::dispatch_file(std::ifstream(L"1.txt"), std::ofstream(L"1.asm"));
	ToAsm::call_masm(L"1");
	return 0;
}