#include "lex.h"
#include <fstream>
#include <iostream>
#include "yacc.h"
#include <unordered_map>
#include "intermediate_code.h"
#include "to_asm.h"

#ifdef _MSC_VER
void init(std::string _filename){
	//Delete old files:
	std::wstring filename = StringToWString(_filename);
	wchar_t now_directory[256];
#else
void init(std::string filename){
	char now_directory[256];
#endif
	GetCurrentDirectory(256, now_directory);

#ifdef _MSC_VER
	wchar_t new_full_path[256];
	std::wstring full_path = now_directory;
#else
	char new_full_path[256];
	std::string full_path = now_directory;
#endif

#ifdef _MSC_VER
	full_path += L"\\output_compile.txt";
	lstrcpyW(new_full_path, full_path.c_str());
#else
	full_path += "\\output_compile.txt";
	strcpy(new_full_path, full_path.c_str());
#endif
	DeleteFile(new_full_path);
	full_path = now_directory;

#ifdef _MSC_VER
	full_path += L"\\" + filename + L".asm";
	lstrcpyW(new_full_path, full_path.c_str());
#else
	full_path += "\\" + filename + ".asm";
	strcpy(new_full_path, full_path.c_str());
#endif
	DeleteFile(new_full_path);
	full_path = now_directory;

#ifdef _MSC_VER
	full_path += L"\\" + filename + L".obj";
	lstrcpyW(new_full_path, full_path.c_str());
#else
	full_path += "\\" + filename + ".obj";
	strcpy(new_full_path, full_path.c_str());
#endif

	DeleteFile(new_full_path);
	full_path = now_directory;
	
#ifdef _MSC_VER
	full_path += L"\\error.txt";
	lstrcpyW(new_full_path, full_path.c_str());
#else
	full_path += "\\error.txt";
	strcpy(new_full_path, full_path.c_str());
#endif

	DeleteFile(new_full_path);
	full_path = now_directory;
	
#ifdef _MSC_VER
	full_path += L"\\" + filename + L".exe";
	lstrcpyW(new_full_path, full_path.c_str());
#else
	full_path += "\\" + filename + ".exe";
	strcpy(new_full_path, full_path.c_str());
#endif
	DeleteFile(new_full_path);
}
int main(int argc, char ** argv)
{
	if (argc <= 1)
	{
		std::cerr << "Fetal: No input file." << std::endl;
		exit(-1);
	}
	else if (argc >= 3)
	{
		std::cout << "Warning: the argument after " << argv[1] << " will be ignored." << std::endl;
	}
	try
	{
		init("main");
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
			std::cout << "Initializing table, please wait..." << std::endl;
            yacc.build_LR1();
#ifdef Yacc_DEBUG
            yacc.print();
#endif
            std::ofstream ofs("saved_table");
            yacc.write_table(ofs);
			std::cout << "Table build success, now you can compile your program!" << std::endl;
            return 0;
		}
        else {
            std::ifstream ifs("saved_table");
            if (!ifs){
				std::cerr << "Please use '--init' to initialize table first." << std::endl;
                exit(-1);
            }
			std::cout << "Table file detected, reading..." << std::endl;
            yacc.read_table(ifs);
			std::cout << "Table read success!" << std::endl;
        }
		std::cout << "Analyzing program and building tuples..." << std::endl;
		Lex program_lex(argv[1]);
		std::vector<token> tokens;
		while (true){
            token temp_token = program_lex.get_token();
            tokens.push_back(temp_token);
            if (temp_token == EOFLINE_TOKEN) {
                break;
            }
		}
		newNode *root = yacc.analyze1(tokens);
		//½¨Á¢·ûºÅ±í
		//get_symbol_table(root);
		Record::add_to_vars(root);

		std::vector<four_tuple> total_buffer;
		Record::translate_expr(total_buffer, root);
		four_tuple temp_end = four_tuple{ "_", "_", "_", "_" };
		total_buffer.push_back(temp_end);
		Record::adjust_jump(total_buffer);
		Record::optimization(total_buffer);
		Record::output_tuples(total_buffer);

		std::cout << "Tuples build success!" << std::endl;
		ToAsm::set_masm32_file();
		std::ifstream ifs("output_compile.txt");
		std::ofstream ofs("main.asm");
		ToAsm::dispatch_file(ifs, ofs);
		ToAsm::call_masm("main");
		return 0;
	}
	catch (File_Error)
	{
		std::cerr << "Fetal: Cannot open file " << argv[1] << std::endl;
		exit(-1);
	}

}
