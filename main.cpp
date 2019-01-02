#include "lex.h"
#include <fstream>
#include <iostream>
#include "yacc.h"
#include<unordered_map>
#include "intermediate_code.h"

int main(int argc, char ** argv)
{
    freopen("analyze.xp", "w", stdout);
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
		Lex grammar_lex("context_free_grammar.txt");
		std::vector<grammar> grammars;
		while (true){
			grammar gm = grammar_lex.get_grammar();
			if (gm == EOF_GRAMMAR)
				break;
			grammars.push_back(gm);
		}
		token start_word = grammars[0].before_word;

		Yacc yacc(grammars, start_word);
		if (strcmp(argv[1], "--init") == 0){
            yacc.build_LR1();
#ifdef Yacc_DEBUG
            yacc.print();
#endif
            std::ofstream ofs("saved_table");
            yacc.write_table(ofs);
            return 0;
		}
        else {
            std::ifstream ifs("saved_table");
            if (!ifs){
                fprintf(stderr, "Please use '--init' to initialize table first.");
                exit(-1);
            }
            yacc.read_table(ifs);
        }

		Lex program_lex(argv[1]);
		std::vector<token> tokens;
		while (true){
            token temp_token = program_lex.get_token();
            tokens.push_back(temp_token);
            if (temp_token == EOFLINE_TOKEN) {
                break;
            }
		}
		std::cout<<"Program Tokens:"<<std::endl;
		std::cout<<tokens<<std::endl;
		newNode *root = yacc.analyze1(tokens);
		//½¨Á¢·ûºÅ±í
		get_symbol_table(root);

		//SYMBOL_TABLE symbol_table = read_symbol_table();
		//for (std::unordered_map<char*,int>::iterator iter = symbol_table.begin(); iter != symbol_table.end(); iter++)
		//{
		//	std::cout << iter->first << "\t" << iter->second << std::endl;
		//}

		std::vector<four_tuple> total_buffer;
		translate_expr(total_buffer, root);
		for (int i = 0; i< total_buffer.size(); i++)
		{
			if (total_buffer[i].op == "jnz" || total_buffer[i].op == "j")
				total_buffer[i].result = std::to_string(std::stoi(total_buffer[i].result) + Record::address_output);
			Record::output_my_four_tuple(total_buffer[i]);
		}
		four_tuple temp = four_tuple{ "_","_","_","_" };
		Record::output_my_four_tuple(temp);

		return 0;
	}
	catch (File_Error)
	{
		fprintf(stderr, "Fetal: Cannot open file %s.\n", argv[1]);
		exit(-1);
	}
}
