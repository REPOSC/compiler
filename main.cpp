#include "lex.h"
#include "yacc.h"
#include <fstream>

int main(int argc, char ** argv)
{

	freopen("1.csv", "w", stdout);
	
	std::string start = "Program";
	std::ifstream isf("context_free_grammar.txt");
	std::vector<std::string> grammar_strs;
	std::string temp_grammar_str;
	while (getline(isf, temp_grammar_str)){
		grammar_strs.push_back(temp_grammar_str);
	}
	
	Yacc y(grammar_strs, start);
	y.build_LR1();
	y.print();

	std::ifstream ifx("1.txt");
	std::string temp_string;
	std::string all_string;
	while (std::getline(ifx, temp_string)) {
		all_string += (temp_string + ' ');
	}
	std::cout << all_string << std::endl;

	Token000 *one = new Token000{ TYPENAME,"void" };
	Token000 *two = new Token000{ TYPENAME,"main" };
	Token000 *three = new Token000{ DELIMITER,"(" };
	Token000 *four = new Token000{ DELIMITER,")" };
	Token000 *five = new Token000{ DELIMITER,"{" };
	Token000 *six = new Token000{ TYPENAME,"int" };
	Token000 *seven = new Token000{ VARNAME,"a" };
	Token000 *eight = new Token000{ OPERATOR,"=" };
	Token000 *nine = new Token000{ INT_NUM,"3" };
	Token000 *ten = new Token000{ DELIMITER,";" };
	Token000 *ele = new Token000{ DELIMITER,"}" };
	TOKEN_SET000 test;
	test.push_back(one);
	test.push_back(two);
	test.push_back(three);
	test.push_back(four);
	test.push_back(five);
	test.push_back(six);
	test.push_back(seven);
	test.push_back(eight);
	test.push_back(nine);
	test.push_back(ten);
	test.push_back(ele);
	//y.analyze(all_string);
	y.analyze000(test);


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
