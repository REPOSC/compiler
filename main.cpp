#include "lex.h"
#include "yacc.h"
#include<fstream>

int main(int argc, char ** argv)
{

	//freopen("1.csv", "w", stdout);
	/*
		std::vector<std::string> t{ "i", "+", "*", "(", ")"};
		std::vector<std::string> u{ "E", "T", "F" };
		std::vector<std::string> g{
			" E-> E + T",
			" E-> T",
			"T-> T *   F ",
			"T-> F    ",
			"F  -> i ",
			"F   ->  ( E    )"
	*/

	//std::vector<std::string> t{};
	//std::vector<std::string> u{};
	std::vector<std::string> g{};
	std::vector<std::string> T{};
	std::vector<std::string> U{};

	std::ifstream fin1(".\\context_free_grammar.txt");
	std::string str1;
	while (getline(fin1, str1))
	{
		std::cout << "G: " << str1 << std::endl;
		g.push_back(str1);
	}
	fin1.close();
	char * temprow = new char[50];
	char * tempchar = new char[20];
	std::vector<std::string>::iterator tempit;
	std::string start;
	bool first = true;
	for (std::string i : g) {
		strcpy(temprow, i.c_str());
		tempchar = strtok(temprow, " ");
		if (std::find(U.begin(),U.end(),tempchar) == U.end()) {
			U.push_back(tempchar);
			if (first) {
				start = tempchar;
				first = false;
			}
		}
		for (tempit = T.begin(); tempit < T.end();tempit ++) {
			if (*tempit == tempchar) {
				T.erase(tempit);
				break;
			}
		}
		tempchar = strtok(NULL, " ");
		tempchar = strtok(NULL, " ");
		while (tempchar != NULL) {
			if (std::find(U.begin(), U.end(), tempchar) == U.end()) {
				if (std::find(T.begin(), T.end(), tempchar) == T.end()) {
					T.push_back(tempchar);
				}
			}
			tempchar = strtok(NULL, " ");
		}
	}
	/*std::cout << "2";
	std::ifstream fin2(".\\U.txt");
	std::string str2;
	while (getline(fin2, str2))
	{
		std::cout << "U: " << str2 << std::endl;
		u.push_back(str2);
	}
	fin2.close();
	std::cout << "3";
	std::ifstream fin3(".\\T.txt");
	std::string str3;
	while (getline(fin3, str3))
	{
		std::cout << "T: " << str3 << std::endl;
		t.push_back(str3);
	}
	fin3.close();

	std::cout << "4";*/

	Yacc y(T, U, start);
	y.build_LR1(g);
	y.print();
	y.analyze("int var ;");



	if (argc <= 1)
	{
		fprintf(stderr, "Fetal: No input file.\n");
		system("pause");
		exit(-1);
	}
	else if (argc >= 3)
	{
		printf("Warning: the argument after %s will be ignored.\n", argv[1]);
	}
	try
	{
		Lex lex(argv[1]);
		token tk = ERR_TOKEN;
		TOKEN_SET one_token_set;
		SYMBOL_TABLE one_symbol_table;
		char * var_name;
		while (tk.type != EOF_TOKEN.type)
		{
			tk = lex.get_token();
			one_token_set.push_back(tk);
			lex.Symbol_Token();
			switch (tk.type)
			{
			case TYPENAME:   printf("【类型表示符】 类型编号：%d\n", tk.value.sym_name); break;
			case INT_NUM:    printf("【整数】       %d\n", tk.value.int_value); break;
			case REAL_NUM:   printf("【浮点数】     %lf\n", tk.value.real_value); break;
			case STRING:     printf("【字符串】     首字符地址：%p\n", tk.value.str_name); break;
			case DELIMITER:  printf("【分隔符】     类型编号：%d\n", tk.value.sym_name); break;
			case CONTROLLER: printf("【控制符】     类型编号：%d\n", tk.value.sym_name); break;
			case OPERATOR:   printf("【运算符】     类型编号：%d\n", tk.value.sym_name); break;
			case MAIN:       printf("【MAIN】\n"); break;
			case ERR_TYPE:   printf("无效字符！\n"); break;
			case EOF_TYPE:   printf("文件结束。\n"); break;
			case VARNAME:
				var_name = one_token_set.back().value.var_name;
				if (one_symbol_table.find(var_name) != one_symbol_table.end())
				{
					printf("【变量名】     变量地址：%p\n", one_symbol_table[var_name]->address);
				}
				else
				{
					VAR_VALUE * one = new VAR_VALUE{ new void *, 0 };
					one_symbol_table[var_name] = one;
					printf("【变量名】     变量地址：%p\n", one_symbol_table[var_name]->address);
				}
				break;
			}
		}
		for (int i = 0; i < one_token_set.size(); i++) {
			token tk = one_token_set[i];
			switch (tk.type)
			{
			case TYPENAME:   printf("【类型表示符】 类型编号：%d\n", tk.value.sym_name); break;
			case INT_NUM:    printf("【整数】       %d\n", tk.value.int_value); break;
			case REAL_NUM:   printf("【浮点数】     %lf\n", tk.value.real_value); break;
			case STRING:     printf("【字符串】     首字符地址：%p\n", tk.value.str_name); break;
			case DELIMITER:  printf("【分隔符】     类型编号：%d\n", tk.value.sym_name); break;
			case CONTROLLER: printf("【控制符】     类型编号：%d\n", tk.value.sym_name); break;
			case OPERATOR:   printf("【运算符】     类型编号：%d\n", tk.value.sym_name); break;
			case MAIN:       printf("【MAIN】\n"); break;
			case ERR_TYPE:   printf("无效字符！\n"); break;
			case EOF_TYPE:   printf("文件结束。\n"); break;
			case VARNAME:
				var_name = one_token_set[i].value.var_name;
				if (one_symbol_table.find(var_name) != one_symbol_table.end())
				{
					printf("【变量名】     变量地址：%p\n", one_symbol_table[var_name]->address);
				}
				else
				{
					VAR_VALUE * one = new VAR_VALUE{ new void *, 0 };
					one_symbol_table[var_name] = one;
					printf("【变量名】     变量地址：%p\n", one_symbol_table[var_name]->address);
				}
				break;
			}
		}
		system("pause");
		return 0;
	}
	catch (File_Error)
	{
		fprintf(stderr, "Fetal: Cannot open file %s.\n", argv[1]);
		exit(-1);
	}
}
