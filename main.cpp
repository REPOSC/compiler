#include "lex.h"
#include <fstream>
#include <iostream>
#include "yacc.h"

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
		//std::cout << "++++++++++++++++" << std::endl;
		//for (auto i : grammars) {
		//	std::cout << i.before_word<<"  "<<i.move << std::endl;
		//}
		//std::cout << "++++++++++++++++" << std::endl;
		token start_word = grammars[0].before_word;

		Yacc yacc(grammars, start_word);
		if (strcmp(argv[1], "--init") == 0){
            yacc.build_LR1();
            yacc.print();
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
		yacc.analyze1(tokens);
		return 0;
	}
	catch (File_Error)
	{
		fprintf(stderr, "Fetal: Cannot open file %s.\n", argv[1]);
		exit(-1);
	}
}
