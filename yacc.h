#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <stack>
#include "token.h"
#include "grammar.h"

//#define Yacc_DEBUG
#define Abstract_TREE

#ifndef YACC__32
#define YACC__32

typedef std::vector<token> receivers;
typedef std::set<token> first_set;
typedef struct { char action; int index; } movement;
typedef std::map<token, std::vector<movement> > table_item;
typedef struct {
	int grammar_index;
	int pointer;
	token forward_word;
} project;
bool operator <(const project & p1, const project & p2) {
	if (p1.grammar_index > p2.grammar_index) return true;
	else if (p1.grammar_index < p2.grammar_index) return false;
	if (p1.pointer > p2.pointer) return true;
	else if (p1.pointer < p2.pointer) return false;
	if (p1.forward_word < p2.forward_word) return true;
	return false;
}

bool operator == (const project & p1, const project & p2) {
	return p1.grammar_index == p2.grammar_index &&
		p1.pointer == p2.pointer &&
		p1.forward_word == p2.forward_word;
}

typedef struct {
	std::vector<project> projects;
	std::map<token, int> convert_to; /* Accept a token, and convert to the statement index */
} statement;

bool operator == (statement & p1, statement & p2) {
	std::sort(p1.projects.begin(), p1.projects.end());
	std::sort(p2.projects.begin(), p2.projects.end());
	return p1.projects == p2.projects;
}

template <class T>
std::ostream & operator << (std::ostream & os, const std::vector<T> & arr) {
	os << "[";
	if (arr.size()) {
		for (int i = 0; i < arr.size() - 1; ++i) {
			os << arr[i] << ",";
		}
		os << arr[arr.size() - 1];
	}
	os << "]";
	return os;
}

struct newNode {
	token onetoken;
	std::vector<newNode *> children;
};

struct newWord {
	token onetoken;
	std::vector<newNode *> children;
};

void write_table_item_to_file(const table_item & ti, std::ofstream & ofs) {
	ofs << ti.size() << std::endl;
	for (auto it : ti) {
		write_token_to_file(it.first, ofs);
		ofs << it.second.size() << " ";
		for (auto mvt : it.second) {
			ofs << mvt.action << mvt.index << " ";
		}
		ofs << std::endl;
	}
}
table_item read_table_item_from_file(std::ifstream & ifs) {
	table_item ti;
	int token_size;
	ifs >> token_size;
	for (int i = 0; i < token_size; ++i) {
		token tk = read_token_from_file(ifs);
		int vec_size;
		ifs >> vec_size;
		std::vector<movement> mvmts;
		for (int j = 0; j < vec_size; ++j) {
			ifs.get();
			movement mvmt;
			mvmt.action = ifs.get();
			ifs >> mvmt.index;
			mvmts.push_back(mvmt);
		}
		ti[tk] = mvmts;
	}
	return ti;
}

class Yacc
{
private:
	token m_start_word;
	std::set<token> m_terminators, m_unterminators;
	std::vector<grammar> m_grammars;
	std::map<token, receivers> m_signal_table;
	std::map<token, first_set> m_firsts;
	std::vector<statement> m_statements;
	std::vector<table_item> m_movement_table;
	void yacc_output_failure(std::string error, grammar gra)
	{
		std::ofstream outfile;
		outfile.open("error.txt");
		// �ٴ����ļ�д���û����������
		outfile << error << "  ";
		outfile << gra.before_word << "->" << gra.after_words << std::endl;
		// �رմ򿪵��ļ�
		outfile.close();
	}


	grammar agm_grammars() {
		/* Augment to current m_grammars */
		grammar agm_g;
		agm_g.after_words.push_back(m_start_word);
		agm_g.before_word = m_start_word = BEGIN_TOKEN;
		agm_g.move = same;
		agm_g.strange_token = EOFLINE_TOKEN;
		return agm_g;
	}

	void get_first(token & current_str, first_set & current_first) {
		/* Reach first set of current_str, and insert them into current_first */
		if (current_str.type == UNTERMINATOR) {
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
				token point_token = m_grammars[stmt.projects[i].grammar_index].after_words[stmt.projects[i].pointer];
				if (point_token.type == UNTERMINATOR) { /* �����ź����Ƿ��ս�� */

														/* �ӵ�ź�����ַ�������һ���ַ���ʼһֱ������ʽ�����������ַ� */
					std::vector<token> after_pointer;
					for (int j = stmt.projects[i].pointer + 1; j < m_grammars[stmt.projects[i].grammar_index].after_words.size(); ++j)
						after_pointer.push_back(m_grammars[stmt.projects[i].grammar_index].after_words[j]);
					after_pointer.push_back(stmt.projects[i].forward_word);
					first_set current_first;
					for (int i = 0; i < after_pointer.size(); ++i) {
						get_first(after_pointer[i], current_first);
						auto find_pos = find(current_first.begin(), current_first.end(), SPC_TOKEN);
						/* ����пհ��ַ������������� */
						if (find_pos == current_first.end()) {
							break;
						}
						else {
							current_first.erase(find_pos);
						}
					};
					/* �������е��Ը÷��ս��Ϊ��ʼ���Ĳ���ʽ */
					for (int i = 0; i < m_grammars.size(); ++i) {
						if (m_grammars[i].before_word == point_token) {
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

	void rr_get_first(const token & unterminator_token) {
		/* Get first set of unterminator_token, and put it into array m_firsts */
		bool changed = false;
		for (auto current_grammar : m_grammars) {
			if (current_grammar.before_word == unterminator_token) {
				token first_word = current_grammar.after_words[0];
				if (first_word.type != UNTERMINATOR) {
					if (std::find(m_firsts[unterminator_token].begin(), m_firsts[unterminator_token].end(), first_word) == m_firsts[unterminator_token].end()) {
						changed = true;
						m_firsts[unterminator_token].insert(first_word);
					}
				}
				else {
					for (auto word : m_firsts[first_word]) {
						if (std::find(m_firsts[unterminator_token].begin(), m_firsts[unterminator_token].end(), word) == m_firsts[unterminator_token].end()) {
							changed = true;
							m_firsts[unterminator_token].insert(word);
						}
					}
				}
			}
		}
		if (changed) {
			for (auto signal_receiver : m_signal_table[unterminator_token]) {
				rr_get_first(signal_receiver);
			}
		}
	}

	void fflush(statement & current_statement) {
		for (project & proj : current_statement.projects) {
			while (proj.pointer < m_grammars[proj.grammar_index].after_words.size() &&
				m_grammars[proj.grammar_index].after_words[proj.pointer] == SPC_TOKEN) {
				++proj.pointer;
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
		typedef struct {    /* A group with the same next recognize_str */
			std::set<project> projects_set;
			token recognize_str;
		} same_set;

		std::vector<same_set> sets;
		for (auto current_project : m_statements[current_index].projects) {
			/* For each project in the statement */
			bool recognized = false;
			if (current_project.pointer < m_grammars[current_project.grammar_index].after_words.size()) {
				for (same_set & current_set : sets) {
					/* See if there is a set with the same recognize_str, if so, add to it */
					if (current_set.recognize_str == m_grammars[current_project.grammar_index].after_words[current_project.pointer]) {
						current_set.projects_set.insert(current_project);
						recognized = true;
						break;
					}
				}
				if (!recognized) { /* If not, create a new set */
					same_set new_set;
					new_set.recognize_str = m_grammars[current_project.grammar_index].after_words[current_project.pointer];
					new_set.projects_set.insert(current_project);
					sets.push_back(new_set);
				}
			}
			else { /* A new same_set containing useless projects(cannot go next step) */
				for (same_set & current_set : sets) {
					if (current_set.recognize_str == EOFLINE_TOKEN) {
						current_set.projects_set.insert(current_project);
						recognized = true;
						break;
					}
				}
				if (!recognized) {
					same_set new_set;
					new_set.recognize_str = EOFLINE_TOKEN;
					new_set.projects_set.insert(current_project);
					sets.push_back(new_set);
				}
			}
		}
		for (same_set & current_set : sets) {
			/* For each same_set, create a statement */
			if (current_set.recognize_str != EOFLINE_TOKEN) {
				statement new_stmt;
				for (auto current_project : current_set.projects_set) {
					project next_project = get_next_proj(current_project);
					new_stmt.projects.push_back(next_project);
				}
				complete_stmt(new_stmt);
				int equal_pos = -1;
				for (int i = 0; i < m_statements.size(); ++i) {
					/* See if the new statement already exists in the m_statements */
					if (new_stmt == m_statements[i]) {
						equal_pos = i;
						break;
					}
				}
				if (equal_pos >= 0)
					m_statements[current_index].convert_to[current_set.recognize_str] = equal_pos;
				else { /* If not, push_back the new statement */
					m_statements[current_index].convert_to[current_set.recognize_str] = m_statements.size();
					m_statements.push_back(new_stmt);
				}
			}
		}
	}

	void build_table() {
		/* Build state transition table by current m_statements, and store it in m_movement_table */
		for (auto statement : m_statements) {
			table_item current_item;
			for (auto cvrt_item : statement.convert_to) {
				token step_token = cvrt_item.first;
				if (step_token.type == UNTERMINATOR) {
					current_item[step_token].push_back({ ' ', cvrt_item.second });
				}
				else {
					current_item[step_token].push_back({ 's', cvrt_item.second });
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
		const std::vector<token> & word_stk,
		const std::vector<token> & word_seq,
		int pointer,
		movement now_movement) {
		std::cout << "Status: " << status_stk << " ";
		std::cout << std::endl;
		std::cout << "Word_stk: " << word_stk << " ";
		std::cout << std::endl;
		std::vector<token> part_word_seq;
		for (int i = pointer; i < word_seq.size(); ++i)
			part_word_seq.push_back(word_seq[i]);
		std::cout << "InputStream: " << part_word_seq << " ";
		std::cout << now_movement.action << now_movement.index << std::endl;
		std::cout << std::endl;
	}
#endif
public:
	Yacc(const std::vector<grammar> & grammars, const token & start_word) {
		/* Initialize terminators, un_terminators and start word */
		m_start_word = start_word;
		grammar begin_grammar = agm_grammars();
		m_grammars.push_back(begin_grammar);

		for (const grammar & current_grammar : grammars) {
			m_grammars.push_back(current_grammar);
		}

		/* Build m_terminators & m_unterminators (Optional) */
		for (auto current_grammar : m_grammars) {
			m_unterminators.insert(current_grammar.before_word);
			for (auto word : current_grammar.after_words) {
				if (word.type != UNTERMINATOR) {
					m_terminators.insert(word);
				}
			}
		}
	}

	void build_LR1() {
		/* Build LR1 table by grammars */
		for (auto current_grammar : m_grammars) {
			m_signal_table[current_grammar.before_word] = receivers();
		}
		for (auto current_grammar : m_grammars) {
			if (current_grammar.after_words[0].type == UNTERMINATOR) {
				m_signal_table[current_grammar.after_words[0]].push_back(current_grammar.before_word);
			}
		}
		for (auto current_grammar : m_grammars) {
			rr_get_first(current_grammar.before_word);
		}

		/* Build the first project */
		project begin_project;
		begin_project.grammar_index = 0;
		begin_project.pointer = 0;
		begin_project.forward_word = EOFLINE_TOKEN;
		statement begin_stmt;
		begin_stmt.projects.push_back(begin_project);
		complete_stmt(begin_stmt);
		m_statements.push_back(begin_stmt);

		/* Build All statements */
		for (int i = 0; i < m_statements.size(); ++i) {
			stmt_walk(i);
		}

		build_table();
	}

	void write_table(std::ofstream & ofs) {
		/* Write built table to file */
		ofs << m_movement_table.size() << std::endl;
		for (auto current_tb_item : m_movement_table) {
			write_table_item_to_file(current_tb_item, ofs);
		}
	}

	void read_table(std::ifstream & ifs) {
		/* Read built table from file */
		m_movement_table.clear();
		int all_size;
		ifs >> all_size;
		for (int i = 0; i < all_size; ++i) {
			table_item ti = read_table_item_from_file(ifs);
			m_movement_table.push_back(ti);
		}
	}

	void clear_LR1() {
		/* Delete built LR1 table and interim status information */
		m_signal_table.clear();
		m_statements.clear();
		m_grammars.clear();
		m_firsts.clear();
		m_movement_table.clear();
	}

	bool check() {
		/* Check if the grammar's not ambigious */
		for (int i = 0; i < m_movement_table.size(); ++i) {
			for (auto j : m_movement_table[i]) {
				if (j.second.size() > 1)
					return false;
			}
		}
		return true;
	}


#ifdef Abstract_TREE
	bool judge_null(newNode *root, int i) {
		for (int j = i + 1; j < root->children.size(); j++) {
			if (root->children[j]->onetoken.type != NULL_TOKEN) {
				return false;
			}
		}
		return true;
	}

	void print_tree(newNode * root, std::string s, bool last) {
		if (!root) {
			return;
		}
		else {
			if (root->onetoken.type != NULL_TOKEN)
				std::cout << s + "  |---" << root->onetoken << std::endl;
			int len = root->children.size();
			for (int i = 0; i < len; i++) {
				if (last) {
					if (judge_null(root, i)) {
						print_tree(root->children[i], s + "      ", true);
					}
					else {
						print_tree(root->children[i], s + "      ", false);
					}
				}
				else {
					if (judge_null(root, i)) {
						print_tree(root->children[i], s + "  |   ", true);
					}
					else {
						print_tree(root->children[i], s + "  |   ", false);
					}
				}

			}
			return;
		}
	}
#endif //Abstract_TREE

	newNode* analyze1(const std::vector<token> & readin_seq) {
		/* Output the process of LR1 derivation */
		//check();
		std::vector<int> status_stk;
		std::vector<token> cout_word_stk;
		std::vector<newWord> word_stk;//����ջ
		std::string now_str;
		std::vector<newNode *> nodes;//�﷨���ڵ�
		int now_status;
		newNode * root = new newNode;
		int pointer = 0;
		status_stk.push_back(0);
		bool finished = false;
		while (!finished) {
			token reserved_str = readin_seq[pointer];
			token now_str;
			if (reserved_str.type == VARNAME)
				now_str.type = ABSTRACT_VAR;
			else if (reserved_str.type == INT_NUM || reserved_str.type == REAL_NUM)
				now_str.type = ABSTRACT_NUM;
			else
				now_str = reserved_str;
			int now_status = status_stk.back();
			if (m_movement_table[now_status].find(now_str) != m_movement_table[now_status].end())
			{
				movement now_movement = m_movement_table[now_status][now_str][0];
#ifdef Yacc_DEBUG
				print_status(status_stk, cout_word_stk, readin_seq, pointer, now_movement);
#endif
				switch (now_movement.action) {
				case 's':
					status_stk.push_back(now_movement.index);
					token onetoken;
					onetoken.type = reserved_str.type;
					onetoken.value = reserved_str.value;
					word_stk.push_back(newWord{ onetoken, });
					cout_word_stk.push_back(reserved_str);
					++pointer;
					break;

				case 'r':
					const grammar & current_grammar = m_grammars[now_movement.index];
					int check_index = current_grammar.after_words.size();
					std::vector<newWord> new_child;
					for (; !status_stk.empty() && !word_stk.empty() && check_index > 0; --check_index) {
						token expected_string = current_grammar.after_words[check_index - 1];
						if (expected_string != SPC_TOKEN) {
							new_child.push_back(word_stk.back());
							status_stk.pop_back();
							word_stk.pop_back();
							cout_word_stk.pop_back();
						}
					}
					if (check_index) {

						yacc_output_failure("ERROR4:", current_grammar);
						std::cerr << std::endl;
						std::cerr << "ERROR occur!" << std::endl;
						exit(-1);
						/*std::cout << "444444 error" << std::endl;
						throw 3;*/
					}
					//reverse(new_child.begin(), new_child.end());
					int child_len = new_child.size();
					now_str = current_grammar.before_word;
					now_status = status_stk.back();
					token temp_token;
					temp_token.type = now_str.type;
					temp_token.value = now_str.value;
					newWord left_word;
					left_word.onetoken = temp_token;
					newNode * temp_node = new newNode;
					switch (current_grammar.move) {
					case makeleaf:
						temp_node->onetoken = new_child.back().onetoken;
						nodes.push_back(temp_node);
						left_word.children.push_back(temp_node);
						break;
					case makenode:
						temp_node->onetoken = current_grammar.strange_token;
						for (int n = 0; n < child_len; n++) {
							if (new_child.back().onetoken.type == UNTERMINATOR) {
								for (auto i : new_child.back().children) {
									temp_node->children.push_back(i);
								}
							}
							new_child.pop_back();
						}
						nodes.push_back(temp_node);
						left_word.children.push_back(temp_node);
						break;
					case same:
						for (int n = 0; n < child_len; n++) {
							if (new_child.back().onetoken.type == UNTERMINATOR) {
								for (auto i : new_child.back().children) {
									left_word.children.push_back(i);
								}
							}
							new_child.pop_back();
						}
						break;
					case null:
						temp_node->onetoken = token{ NULL_TOKEN, };
						nodes.push_back(temp_node);
						left_word.children.push_back(temp_node);
						break;
					}

					if (now_str == m_start_word && status_stk.size() == 1 && status_stk.back() == 0) {
						root = left_word.children[0];
						finished = true;
					}
					else {
						word_stk.push_back(left_word);
						cout_word_stk.push_back(left_word.onetoken);
						if (m_movement_table[now_status].find(now_str) != m_movement_table[now_status].end()) {
							now_movement = m_movement_table[now_status][now_str][0];
							if (now_movement.action != ' ') {
								yacc_output_failure("ERROR2:", current_grammar);
								std::cerr << std::endl;
								std::cerr << "ERROR occur!" << std::endl;
								exit(-1);
								/*std::cout << "222222 error" << std::endl;
								throw 3;*/
							}
							else status_stk.push_back(now_movement.index);
						}
						else {
							yacc_output_failure("ERROR3:", current_grammar);
							std::cerr << std::endl;
							std::cerr << "ERROR occur!" << std::endl;
							exit(-1);
							/*std::cout << "333333 error" << std::endl;
							throw 3;*/
						}
					}
					break;
				}
			}
			else {
				yacc_output_failure("ERROR1:", m_grammars[0]);
				std::cerr << std::endl;
				std::cerr << "ERROR occur!" << std::endl;
				exit(-1);
				/*std::cout << "111111 error" << std::endl;
				std::cout << now_status << " " << now_str;
				throw 1;*/
			}
		}
#ifdef Abstract_TREE
		std::cout << std::endl;
		std::cout << "Abstract Tree:" << std::endl;
		print_tree(root, "", true);
#endif // Abstract_TREE
		return root;
	}

#ifdef Yacc_DEBUG
	void print() {
		/* Output some useful information */
		std::cout << "signals:" << std::endl;
		for (auto i : m_signal_table) {
			std::cout << i.first << ":" << std::endl;
			for (auto j : i.second) {
				std::cout << j << std::endl;
			}
		}
		std::cout << "grammar:" << std::endl;
		for (auto i : m_grammars) {
			std::cout << i.before_word << "->";
			for (auto j : i.after_words) {
				std::cout << j << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "first:" << std::endl;
		for (auto i : m_firsts) {
			std::cout << i.first << ":" << std::endl;
			for (auto j : i.second) {
				std::cout << j << std::endl;
			}
			std::cout << std::endl;
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
		std::vector<token> all_word;
		for (auto word : m_terminators) {
			std::cout << word << ",";
		}
		for (auto word : m_unterminators) {
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
			for (auto word : m_unterminators) {
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
#endif // YACC__32
