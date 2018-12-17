#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <stack>
#include "base.h"

#define UNTERMINAL 9
#define Yacc_DEBUG

typedef struct {
	SYM_TYPE type;
	char * value;
}Token000;

typedef struct Node000 {
	int index;
	Token000 token;
	char * syn;
	char * inh;
	std::vector<int> children;
};

bool operator ==(Node000 &a, Node000 &b) {
	return a.index == b.index;
}

typedef struct {
	Token000 token;
	int index;
}OneWord;

typedef std::vector<Token000> TOKEN_SET000;
typedef std::vector<Node000 *> NODE_SET000;

std::string get_string_tk(Token000 tk) {
	if (tk.type == 0)
		return "var";
	else if (tk.type == 1 || tk.type == 2)
		return "num";
	else
		return tk.value;
}




typedef std::vector<std::string> receivers;

typedef struct {
	std::string before_word;
	std::vector<std::string> after_words;
} grammar;	

typedef std::set<std::string> first_set;	

typedef struct {
	int grammar_index;
	int pointer;
	std::string forward_word;
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
	std::unordered_map<std::string, int> convert_to;
} statement;

bool operator == (statement & p1, statement & p2) {
	std::sort(p1.projects.begin(), p1.projects.end());
	std::sort(p2.projects.begin(), p2.projects.end());
	return p1.projects == p2.projects;
}

template <class T>
std::ostream & operator << (std::ostream & os, const std::vector<T> & arr) {
	if (arr.size() <= 0) return os;
	for (int i = 0; i < arr.size() - 1; ++i) {
		os << arr[i] << ",";
	}
	os << arr[arr.size() - 1];
	return os;
}

typedef struct { char action; int index; } movement;
typedef std::unordered_map<std::string, std::vector<movement> > table_item;
typedef std::vector<std::string> word_seq;
typedef struct {} invalid_start_word;
typedef struct { int index; int pointer; } invalid_2nd_grammar;
typedef struct { int statement_index; table_item error_item; } ambiguous_2nd_grammar;
typedef struct { int index; } invalid_input_string;
typedef struct { int index; std::string key; } wrong_table_item;
class Yacc
{
private: 
	const std::string eof_str = "#";
	const std::string spc_str = "space";
	std::set<std::string> m_terminators;
	std::set<std::string> m_un_terminators;
	std::string m_start_word;
	std::vector<grammar> m_grammars;	
	std::unordered_map<std::string, receivers> m_signal_table;
	std::unordered_map<std::string, first_set> m_firsts;
	std::vector<statement> m_statements;
	std::vector<table_item> m_movement_table;

	bool is_sep(char x) { return x == ' ' || x == '\t' || x == '\f'; };
	bool is_end(char x) { return x == '\n' || x == '\0'; }
	bool is_alpha(char x) { return isalpha(x); }

	word_seq get_seq_from_str(const std::string & str, int begin_index) {
		/* Split string into small ones */
		std::string temp_word;
		word_seq result;
		while (is_sep(str[begin_index])) ++begin_index;
		while (!is_end(str[begin_index])) {
			if (is_sep(str[begin_index])) {
				result.push_back(temp_word);
				++begin_index;
				temp_word = "";
				while (is_sep(str[begin_index])) ++begin_index;
			}
			else temp_word += str[begin_index++];
		}
		if (temp_word[0]) result.push_back(temp_word);
		return result;
	}

	grammar get_grammar(const std::string & grammar_str, int index) {
		/* Parse grammar from grammar string */
		grammar result;	
		int pointer = 0;
		while (is_sep(grammar_str[pointer])) ++pointer;
		int before_arrow = grammar_str.find("->");
		if (before_arrow != std::string::npos){
			while (pointer != before_arrow && !is_sep(grammar_str[pointer])) result.before_word += grammar_str[pointer++];
			while (pointer < before_arrow) ++pointer;
			pointer += sizeof("->") / sizeof(char);
			if (result.before_word.size() <= 0) throw invalid_2nd_grammar{ index, pointer };
			result.after_words = get_seq_from_str(grammar_str, pointer);
			if (result.after_words.size() <= 0) throw invalid_2nd_grammar{ index, pointer };
			return result;
		}
		else throw invalid_2nd_grammar{ index, pointer };
	}

	grammar agm_grammars() {
		/* Augment to current m_grammars */
		grammar agm_g;		
		agm_g.after_words.push_back(m_start_word);
		m_start_word = agm_g.before_word = "$$$";
		return agm_g;
	}

	void get_first(std::string & current_str, first_set & current_first) {
		/* Get first set of current_str, and insert them into current_first */
		if (std::find(m_un_terminators.begin(), m_un_terminators.end(), current_str) != m_un_terminators.end()) {
			for (auto first_word : m_firsts[current_str]) {
				current_first.insert(first_word);
			}
		}
		else current_first.insert(current_str);
	}

	void complete_stmt(statement & stmt) {
		/* Complete a statement by deploy projects recursively */
		for (int i = 0; i < stmt.projects.size(); ++i) {
			if (stmt.projects[i].pointer < m_grammars[stmt.projects[i].grammar_index].after_words.size()) {
				std::string point_str = m_grammars[stmt.projects[i].grammar_index].after_words[stmt.projects[i].pointer];	
				if (std::find(m_un_terminators.begin(), m_un_terminators.end(), point_str) != m_un_terminators.end()) {
					std::vector<std::string> after_pointer;
					for (int j = stmt.projects[i].pointer + 1; j < m_grammars[stmt.projects[i].grammar_index].after_words.size(); ++j)
						after_pointer.push_back(m_grammars[stmt.projects[i].grammar_index].after_words[j]);
					after_pointer.push_back(stmt.projects[i].forward_word);
					first_set current_first;
					for (int i = 0; i < after_pointer.size(); ++i) {
						get_first(after_pointer[i], current_first);
						auto find_pos = find(current_first.begin(), current_first.end(), spc_str);
						if (find_pos == current_first.end()) break;
						else current_first.erase(find_pos);	
					}
					for (int i = 0; i < m_grammars.size(); ++i) {
						if (m_grammars[i].before_word == point_str) {
							for (auto current_forward : current_first) {
								project current_project;
								current_project.grammar_index = i;
								current_project.pointer = 0;
								current_project.forward_word = current_forward;
								if (find(stmt.projects.begin(), stmt.projects.end(), current_project) == stmt.projects.end()) {
									stmt.projects.push_back(current_project);
								}
							}
						}
					}
				}
			}
		}
		fflush(stmt);
	}

	void rr_get_first(const std::string & un_terminator) {
		/* Get first set of un_terminator, and put it into array m_firsts */
		bool changed = false;
		for (auto current_grammar : m_grammars) {
			if (current_grammar.before_word == un_terminator){
				std::string first_word = current_grammar.after_words[0];
				if (std::find(m_un_terminators.begin(), m_un_terminators.end(), first_word) == m_un_terminators.end()) {
					if (std::find(m_firsts[un_terminator].begin(), m_firsts[un_terminator].end(), first_word) == m_firsts[un_terminator].end()) {
						changed = true;
						m_firsts[un_terminator].insert(first_word);
					}
				}
				else {
					for (auto word: m_firsts[first_word]){
						if (std::find(m_firsts[un_terminator].begin(), m_firsts[un_terminator].end(), word) == m_firsts[un_terminator].end()) {
							changed = true;
							m_firsts[un_terminator].insert(word);
						}
					}
				}
			}
		}
		if (changed) {
			for (auto signal_receiver : m_signal_table[un_terminator]) {
				rr_get_first(signal_receiver);
			}
		}
	}

	project get_next_proj(const project & proj) {
		/* Get project of current project with pointer moved forward */
		project new_proj = proj;
		if (proj.pointer < m_grammars[proj.grammar_index].after_words.size()) {
			++new_proj.pointer;
		}
		return new_proj;
	}

	void stmt_walk(int current_index) {
		/* Looking for new states that can be reached in one step under the current state, then deploy the new states */
		typedef struct {
			std::set<project>  projects_set;
			std::string recognize_str;
		} same_set;	
		std::vector<same_set> sets;	
		for (auto current_project: m_statements[current_index].projects) {
			bool recognized = false;
			if (current_project.pointer < m_grammars[current_project.grammar_index].after_words.size()) {
				for (same_set & current_set : sets) {
					if (current_set.recognize_str == m_grammars[current_project.grammar_index].after_words[current_project.pointer]) {
						current_set.projects_set.insert(current_project);
						recognized = true;
						break;
					}
				}
				if (!recognized) {
					same_set new_set;
					new_set.recognize_str = m_grammars[current_project.grammar_index].after_words[current_project.pointer];
					new_set.projects_set.insert(current_project);
					sets.push_back(new_set);
				}
			}
			else {
				for (same_set & current_set : sets) {
					if (current_set.recognize_str == eof_str) {
						current_set.projects_set.insert(current_project);
						recognized = true;
						break;
					}
				}
				if (!recognized) {
					same_set new_set;
					new_set.recognize_str = eof_str;
					new_set.projects_set.insert(current_project);
					sets.push_back(new_set);
				}
			}
		}
		for (same_set & current_set : sets) {
			if (current_set.recognize_str != eof_str) {
				statement new_stmt;
				for (auto current_project : current_set.projects_set) {
					project next_project = get_next_proj(current_project);
					new_stmt.projects.push_back(next_project);
				}
				complete_stmt(new_stmt);
				int equal_pos = -1;	
				for (int i = 0; i < m_statements.size(); ++i) {
					if (new_stmt == m_statements[i]) {
						equal_pos = i;
						break;
					}
				}
				if (equal_pos >= 0) 
					m_statements[current_index].convert_to[current_set.recognize_str] = equal_pos;
				else {
					m_statements[current_index].convert_to[current_set.recognize_str] = m_statements.size();
					m_statements.push_back(new_stmt);
				}
			}
		}			
	}

	void build_table() {
		/* Build tate transition table by current m_statements, and store it in m_movement_table */
		for (auto statement : m_statements) {
			table_item current_item;
			for (auto terminator : m_terminators) {
				if (statement.convert_to.find(terminator) != statement.convert_to.end()) {
					current_item[terminator].push_back({ 's',statement.convert_to[terminator] });
				}				
			}
			for (auto un_terminator : m_un_terminators) {
				if (statement.convert_to.find(un_terminator) != statement.convert_to.end()) {
					current_item[un_terminator].push_back({ ' ',statement.convert_to[un_terminator] });
				}
			}
			for (auto current_proj : statement.projects) {
				if (current_proj.pointer == m_grammars[current_proj.grammar_index].after_words.size()) {
					current_item[current_proj.forward_word].push_back({ 'r', current_proj.grammar_index });
				}
			}
			m_movement_table.push_back(current_item);
		}
	}
	
	void fflush(statement & current_statement){
		for (project & proj: current_statement.projects){
			while (proj.pointer < m_grammars[proj.grammar_index].after_words.size() && 
				m_grammars[proj.grammar_index].after_words[proj.pointer] == spc_str){
				++ proj.pointer;
			}
		}
	}
	
#ifdef Yacc_DEBUG
	void proj_prt(const project & proj) {
		/* Print a project */
		std::cout << m_grammars[proj.grammar_index].before_word << "->";
		for (int i = 0; i < m_grammars[proj.grammar_index].after_words.size(); ++i) {			
			if (i == proj.pointer) {
				std::cout << ".";
			}
			std::cout << m_grammars[proj.grammar_index].after_words[i] << " ";
		}
		if (m_grammars[proj.grammar_index].after_words.size() == proj.pointer) 
			std::cout << ".";
		std::cout << "[" << proj.forward_word << "]" << std::endl;
	}
	void print_status(const std::vector<int> & status_stk,
		const std::vector<std::string> & word_stk,
		const std::vector<std::string> & word_seq,
		int pointer,
		movement now_movement) {
		std::cout << "Status:[" << status_stk << "]" << " ";
		std::cout << "Word_stk:[" << word_stk << "]" << " ";
		std::vector<std::string> part_word_seq;
		for (int i = pointer; i < word_seq.size(); ++i)
			part_word_seq.push_back(word_seq[i]);
		std::cout << "InputStream:[" << part_word_seq << "]" << " ";
		std::cout << now_movement.action << now_movement.index << std::endl;
	}
#endif
public:
	Yacc(const std::vector<std::string> & grammar_strs, const std::string & start_word) {
		/* Initialize terminators, un_terminators and start word */
		m_start_word = start_word;
		grammar begin_grammar = agm_grammars();
		m_grammars.push_back(begin_grammar);

		for (int i = 0; i < grammar_strs.size(); ++i){
			grammar current_grammar = get_grammar(grammar_strs[i], i);
			m_grammars.push_back(current_grammar);
			m_un_terminators.insert(current_grammar.before_word);
		}
		m_terminators.insert(eof_str);
		
		for (auto current_grammar: m_grammars){
			for (auto current_word: current_grammar.after_words){
				if (find(m_un_terminators.begin(), m_un_terminators.end(), current_word) == m_un_terminators.end())
					m_terminators.insert(current_word);
			}
		}
		if (std::find(m_un_terminators.begin(), m_un_terminators.end(), start_word) == m_un_terminators.end()) 
			throw invalid_start_word();
	}

	void build_LR1() {
		/* Build LR1 table by grammars */
		for (auto word : m_un_terminators) {
			m_signal_table[word] = receivers();
		}
		for (auto current_grammar : m_grammars) {
			if (std::find(m_un_terminators.begin(), m_un_terminators.end(), current_grammar.after_words[0]) != m_un_terminators.end()) {
				m_signal_table[current_grammar.after_words[0]].push_back(current_grammar.before_word);
			}
		}
		for (auto word : m_un_terminators) {
			rr_get_first(word);
		}

		//Build the first project
		project begin_project;
		begin_project.grammar_index = 0;
		begin_project.pointer = 0;
		begin_project.forward_word = eof_str;
		statement begin_stmt;
		begin_stmt.projects.push_back(begin_project);
		complete_stmt(begin_stmt);
		m_statements.push_back(begin_stmt);

		for (int i = 0; i < m_statements.size(); ++i) {
			stmt_walk(i);
		}

		build_table();
	}

	void clear_LR1() {
		/* Delete built LR1 table and interim status information */
		m_signal_table.clear();
		m_statements.clear();
		m_grammars.clear();
		m_firsts.clear();
		m_movement_table.clear();
	}
	
	void check() {
		/* Check if the grammar's ambigious */
		for (int i = 0; i < m_movement_table.size(); ++i) {
			for (auto j : m_movement_table[i]) {
				if (j.second.size() > 1)
					throw ambiguous_2nd_grammar({ i, m_movement_table[i]});
			}			
		}
	}



	void analyze(const std::string & readin_str) {
		/* Output the process of LR1 derivation */
		//check();
		word_seq readin_seq = get_seq_from_str(readin_str, 0);
		if (readin_seq.back() != eof_str)
			readin_seq.push_back(eof_str);
		for (int i = 0; i < readin_seq.size(); ++i) {
			if (std::find(m_un_terminators.begin(), m_un_terminators.end(), readin_seq[i]) == m_un_terminators.end()
				&& std::find(m_terminators.begin(), m_terminators.end(), readin_seq[i]) == m_terminators.end())
				throw invalid_input_string{ i };
		}
		std::vector<int> status_stk;
		std::vector<std::string> word_stk;
		int pointer = 0;
		status_stk.push_back(0);

		bool finished = false;
		while (!finished) {
			std::string now_str = readin_seq[pointer];
			int now_status = status_stk.back();	
			if (m_movement_table[now_status].find(now_str) != m_movement_table[now_status].end())
			{
				movement now_movement = m_movement_table[now_status][now_str][0];
#ifdef Yacc_DEBUG
				print_status(status_stk, word_stk, readin_seq, pointer, now_movement);
#endif
				switch (now_movement.action) {
				case 's':					
					status_stk.push_back(now_movement.index);					
					word_stk.push_back(now_str);
					++pointer;
					break;
				case 'r':
					const grammar & current_grammar = m_grammars[now_movement.index];
					int check_index = current_grammar.after_words.size();
					for (; !status_stk.empty() && !word_stk.empty() && check_index > 0; --check_index) {
						std::string expected_string = current_grammar.after_words[check_index - 1];
						if (expected_string != spc_str) {							
							if ( word_stk.back() != expected_string) {
								throw wrong_table_item({ now_status, now_str });
							}
							status_stk.pop_back();
							word_stk.pop_back();
						}							
					}
					if (check_index) throw wrong_table_item({ now_status, now_str });
					now_str = current_grammar.before_word;
					now_status = status_stk.back();
					if (now_str == m_start_word && status_stk.size() == 1 && status_stk.back() == 0)
						finished = true;
					else {
						word_stk.push_back(now_str);
						if (m_movement_table[now_status].find(now_str) != m_movement_table[now_status].end()) {
							now_movement = m_movement_table[now_status][now_str][0];
							if (now_movement.action != ' ') 
								throw wrong_table_item({ now_status, now_str });
							else status_stk.push_back(now_movement.index);
						}
						else throw wrong_table_item({ now_status, now_str });
					}					
					break;
				}
			}
			else
				throw wrong_table_item({ now_status, now_str });			
		}
	}

	void analyze000(TOKEN_SET000 token_set) {
		std::vector<int> status_stk;
		std::vector<OneWord> word_stk;
		std::string now_str;
		NODE_SET000 node_set;//节点集合，语法树中所有的节点
		int now_status;
		int pointer = 0;
		int root;
		status_stk.push_back(0);
		bool finished = false;
		while (!finished) {
			Token000 nowtoken = token_set[pointer];
			now_status = status_stk.back();
			now_str = get_string_tk(nowtoken);
			if (m_movement_table[now_status].find(now_str) != m_movement_table[now_status].end())
			{
				movement now_movement = m_movement_table[now_status][now_str][0];
				//#ifdef Yacc_DEBUG
				//				print_status(status_stk, word_stk, readin_seq, pointer, now_movement);
				//#endif
				switch (now_movement.action) {
				case 's':
					status_stk.push_back(now_movement.index);
					word_stk.push_back(OneWord{ nowtoken,-1 });
					++pointer;
					break;
				case 'r':
					const grammar & current_grammar = m_grammars[now_movement.index];
					int check_index = current_grammar.after_words.size();
					std::vector<int> new_child;
					bool flagone = false;//判断是不是操作符
					bool flagtwo = false;//判断是不是操作符的孩子
					for (; !status_stk.empty() && !word_stk.empty() && check_index > 0; --check_index) {
						Node000 *new_node = new Node000;
						if (word_stk.back().index == -1) {
							new_node->index = node_set.size();
							new_node->token = word_stk.back().token;
							if (new_node->token.type == OPERATOR) {
								char * temp_value = new_node->token.value;
								if (temp_value == "*" || temp_value == "/" || temp_value == "+" || temp_value == "*"
									|| temp_value == "^" || temp_value == "%" || temp_value == ">" || temp_value == "<"
									|| temp_value == ">=" || temp_value == "<=" || temp_value == "==" || temp_value == "!=") {
									if (check_index == 1) {
										throw invalid_input_string{ 0 };
									}
									else {
										std::vector<int> temp_child;
										temp_child.push_back(new_node->index - 1);
										temp_child.push_back(new_node->index + 1);
										new_child.pop_back();
										new_node->children = temp_child;
										flagone = true;
									}
								}
							}
							node_set.push_back(new_node);
						}
						else {
							new_node = node_set[word_stk.back().index];
						}
						std::cout << new_node->token.value << std::endl;
						if (!flagtwo) {
							new_child.push_back(new_node->index);
						}
						else {
							flagtwo = false;
						}
						if (flagone) {
							flagtwo = true;
							flagone = false;
						}
						status_stk.pop_back();
						std::string current_string = word_stk.back().token.value;
						word_stk.pop_back();
						if (current_grammar.after_words[check_index - 1] != current_string) throw wrong_table_item({ now_status, now_str });
					}
					if (check_index) throw wrong_table_item({ now_status, now_str });
					now_str = current_grammar.before_word;
					now_status = status_stk.back();
					char * temp_str = new char[50];
					strcpy(temp_str, now_str.c_str());
					Token000 temptoken{ UNTERMINAL, temp_str };
					Node000 *new_node = new Node000;
					new_node->index = node_set.size();
					new_node->token = temptoken;
					new_node->children = new_child;
					node_set.push_back(new_node);
					if (now_str == m_start_word && status_stk.size() == 1 && status_stk.back() == 0) {
						root = new_node->index;
						finished = true;
					}
					else {
						word_stk.push_back(OneWord{ temptoken, new_node->index });
						if (m_movement_table[now_status].find(now_str) != m_movement_table[now_status].end()) {
							now_movement = m_movement_table[now_status][now_str][0];
							if (now_movement.action != ' ') throw wrong_table_item({ now_status, now_str });
							else status_stk.push_back(now_movement.index);
						}
						else throw wrong_table_item({ now_status, now_str });
					}
					break;
				}
			}
			else
				throw wrong_table_item({ now_status, now_str });
		}

#ifdef Yaacc_DEBUG
		for (int i = 0; i < node_set.size();i++) {
			std::cout << i << " : ";
			char * temp_value = node_set[i]->token.value;
			switch (node_set[i]->token.type) {
			case VARNAME:
				std::cout << std::setw(25) << "ID Declaration" << "symbol:" << std::setw(8)<< temp_value << "Children: ";
				break;
			case INT_NUM:
				std::cout << std::setw(25) << "Const Declaration" << std::setw(15) << temp_value << "Children: ";
				break;
			case REAL_NUM:
				std::cout << std::setw(25) << "Real Const Declaration" << std::setw(15) << temp_value << "Children: ";
				break;
			case TYPENAME:
				std::cout << std::setw(25) << "Type Specifier" << std::setw(15) << temp_value << "Children: ";
				break;
			case OPERATOR:
				std::cout << std::setw(25) << "Expr" << "op:"<<std::setw(12) << temp_value << "Children: ";
				break;
			case CONTROLLER:
				std::cout << std::setw(25) << "Controller" << std::setw(15) << temp_value << "Children: ";
				break;
			case DELIMITER:
				std::cout << std::setw(25) << "Delimiter" << std::setw(15) << temp_value << "Children: ";
				break;
			case UNTERMINAL:
				if(temp_value == "WhileStmt" || temp_value == "ForStmt" || temp_value == "DoStmt")
					std::cout << std::setw(25) << "RepeatK statement" << std::setw(15) << temp_value << "Children: ";
				else if(temp_value == "AssignStmt") 
					std::cout << std::setw(25) << "Assign statement" << std::setw(15) << temp_value << "Children: ";
				else
					std::cout << std::setw(25) << "CompoundK statement" << std::setw(15) << temp_value << "Children: ";
				break;
			default:
				std::cout << std::setw(25) << "CompoundK statement" << std::setw(15) << temp_value << "Children: ";
			}
			for (auto c : node_set[i]->children) {
				std::cout << c << " ";
			}
			std::cout << std::endl;
		}
#endif // Yacc_DEBUG
	}



#ifdef Yacc_DEBUG
	void print(){
		/* Output some useful information */
		std::cout<<"signals:"<<std::endl;
		for (auto i : m_signal_table){
			std::cout<<i.first<<":"<<std::endl;
			for (auto j: i.second){
				std::cout<<j<<std::endl;
			}
		}
		std::cout<<"grammar:"<<std::endl;
		for (auto i : m_grammars){
			std::cout<<i.before_word<<"->";
			for (auto j : i.after_words){
				std::cout<<j<<" ";
			}
			std::cout<<std::endl;
		}
		std::cout << "m_terminators" << std::endl;
		for (auto i : m_terminators) {
			std::cout << i << std::endl;
		}
		std::cout << "m_un_terminators" << std::endl;
		for (auto i : m_un_terminators) {
			std::cout << i << std::endl;
		}
		std::cout<<"first:"<<std::endl;
		for (auto i : m_firsts){
			std::cout<<i.first<<":"<<std::endl;
			for (auto j: i.second){
				std::cout<<j<<std::endl;
			}
			std::cout<<std::endl;
		}
		std::cout << "statement:" << m_statements.size() << std::endl;
		for (int i = 0; i < m_statements.size(); ++i) {
			std::cout << "I" << i << std::endl;
			for (auto current_proj : m_statements[i].projects) {
				proj_prt(current_proj);
			}
		}
		std::cout << "Table:" << std::endl;
		std::cout << " ,";
		for (auto word : m_terminators) {
			std::cout << word << ",";
		}
		for (auto word : m_un_terminators) {
			std::cout << word << ",";
		}
		std::cout << std::endl;
		for (int i = 0; i < m_statements.size(); ++i) {
			std::cout << "I" << i << ",";
			for (auto word : m_terminators) {
				if (m_movement_table[i].find(word) != m_movement_table[i].end()) {
					for (int j = 0; j < m_movement_table[i][word].size(); ++j) {
						std::cout << m_movement_table[i][word][j].action << m_movement_table[i][word][j].index;
						if (j != m_movement_table[i][word].size() - 1)
							std::cout << "/";
					}
					std::cout << ",";
				}
				else {
					std::cout << " ,";
				}
			}
			for (auto word : m_un_terminators) {
				if (m_movement_table[i].find(word) != m_movement_table[i].end()) {
					for (int j = 0; j < m_movement_table[i][word].size(); ++j) {
						std::cout << m_movement_table[i][word][j].action << m_movement_table[i][word][j].index;
					}
					std::cout << ",";
				}
				else {
					std::cout << " ,";
				}
			}
			std::cout << std::endl;
		}
	}
#endif
};