#include <fstream>
#include <iostream>
#include <assert.h>
#include "token.h"
#include "yacc.h"
#include "type.h"
#include "judgement.h"
#include "cal.h"
#ifndef ERROR
#define ERROR -1
#endif

#ifndef INTERMEDIATE_CODE
#define INTERMEDIATE_CODE 
struct four_tuple{
	std::string arg1;
	std::string arg2;
	std::string result;
	std::string op;
	four_tuple(std::string op, std::string arg1, std::string arg2, std::string result){
		this->arg1 = arg1;
		this->arg2 = arg2;
		this->op = op;
		this->result = result;
	}
	four_tuple() {

	}
};

class Record
{
	static int address;
	static four_tuple generate_no_cond_jump(int address);
	static four_tuple generate_binary_operator(std::string my_binary_operator, std::string arg1, std::string arg2, std::vector<four_tuple> &);
	static four_tuple generate_unary_operator(std::string my_binary_operator, std::string arg1);
	static four_tuple generate_scanf(std::string input);
	static four_tuple generate_printf(std::string output);
	static four_tuple generate_int(std::string variable_name);
	static four_tuple generate_double(std::string variable_name);
	static std::string get_next_temp_var(const std::string &, const std::string &, std::vector<four_tuple> &);
	static void output_failure(std::string error);
	static bool has_real(const std::string &, const std::string &);
	static bool is_real(const std::string &);
	static std::string create_temp_variable_unary(std::vector<four_tuple> &buffer_tuple, newNode *node_expr, std::string opera);
	static void traverse_buffer(std::vector<four_tuple> & buffer_tuple, std::vector<four_tuple> & traversed_tuple);
	static token create_name_token1(SYM_TYPE type, const char * name);
	static bool judge_strange_token(newNode *node_expr);
	static bool judge_null_token(newNode *node_expr);
	static void generate_declaration(std::vector<four_tuple> &buffer_tuple, newNode *declaration, std::string variable_name);
	static std::map<std::string, unsigned> symbol_table;
	/*！——这里存放所有变量的类型，临时变量以t开头，一般变量用_开头
	*/
	
	static std::string translate_if(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static std::string translate_var_declaration(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static std::string translate_conpound(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static std::string translate_do(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static std::string translate_while(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static std::string translate_for(std::vector<four_tuple> &buffer_tuple, newNode *node_for);
	static std::string judge_type_to_string(newNode* node_exprs);
	static std::string translate_scanf(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static std::string translate_printf(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs);
	static token get_left_token(newNode * root);
	static bool type_check_two_variable(newNode* node1, newNode* node2);
	static std::string create_temp_variable_binary(std::vector<four_tuple> &buffer_tuple, newNode *node_expr, std::string opera);
public:
	static int address_output;
	static int temp_count;//临时变量计数
	static void optimization(std::vector<four_tuple> & buffer_tuple);
	static std::string translate_expr(std::vector<four_tuple> &buffer_tuple, newNode *node_expr);
	static void add_to_vars(newNode * root);
	static void output_tuples(const std::vector<four_tuple> &);
	static void adjust_jump(std::vector<four_tuple> &);
};

int Record::address = 0;
int Record::address_output = 0;
std::map<std::string, unsigned> Record::symbol_table;
int Record::temp_count = -1;

std::string Record::create_temp_variable_unary(std::vector<four_tuple> &buffer_tuple, newNode *node_expr, std::string opera){
	std::string temp_variable1 = "";
	if (judge_strange_token(node_expr->children[0])){
		temp_variable1 = translate_expr(buffer_tuple, node_expr->children[0]);
	}
	if (temp_variable1 != ""){
		four_tuple temp = Record::generate_unary_operator(opera, temp_variable1);
		buffer_tuple.push_back(temp);
	}
	else{
		four_tuple temp = Record::generate_unary_operator(opera, judge_type_to_string(node_expr->children[0]));
		buffer_tuple.push_back(temp);
	}
	return "t" + std::to_string(Record::temp_count);
}

bool inline Record::is_real(const std::string & s){
	if (check_if_leu(s[0])){
		if (symbol_table.find(s) == symbol_table.end())
			throw ERROR;
		return symbol_table[s] == REAL_NUM;
	}
	for (auto it : s){
		if (it == '.')
			return true;
		else if (!check_if_number(it))
			throw ERROR;
	}
	return false;
}

bool inline Record::has_real(const std::string & s1, const std::string & s2){
	return is_real(s1) || is_real(s2);
}

four_tuple Record::generate_no_cond_jump(int address){
	return four_tuple{ "j", "_", "_", std::to_string(address) };
	//动态分配较好，否则这个值是拷贝传回,浪费
}

four_tuple Record::generate_binary_operator(std::string my_binary_operator, std::string arg1, std::string arg2, std::vector<four_tuple> & buffer_tuple){
	if (my_binary_operator != "=")
		return four_tuple{ my_binary_operator, arg1, arg2, get_next_temp_var(arg1, arg2, buffer_tuple) };
	else
		return four_tuple{ "=", arg2, "_", arg1 };
}

four_tuple Record::generate_unary_operator(std::string my_unary_operator, std::string arg1){
	return four_tuple{ my_unary_operator, arg1, "_", "t" + std::to_string(++Record::temp_count) };
}

//(s,a,_,_)
four_tuple Record::generate_scanf(std::string input){
	return four_tuple{ "s", input, "_", "_" };
}
//(p,a,_,_)
four_tuple Record::generate_printf(std::string output){
	return four_tuple{ "p", output, "_", "_" };
}
//(i,i,_,2)
four_tuple Record::generate_int(std::string variable_name){
	symbol_table[variable_name] = INT_NUM;
	return four_tuple{ "i", variable_name, "_", "_" };
}
//(d,b,_,_)
four_tuple Record::generate_double(std::string variable_name){
	symbol_table[variable_name] = REAL_NUM;
	return four_tuple{ "d", variable_name, "_", "_" };
}

void Record::output_tuples(const std::vector<four_tuple> & tuples){
	std::ofstream of("output_compile.txt");
	for (auto it : tuples){
		of << "(" << it.op << "," << it.arg1 << "," << it.arg2 << "," << it.result << ")" << std::endl;
	}
	of.close();
}

void Record::output_failure(std::string error)
{
	std::ofstream outfile;
	outfile.open("error.txt", std::ios::app);
	// 再次向文件写入用户输入的数据
	outfile << error << std::endl;
	// 关闭打开的文件
	outfile.close();
}

void Record::adjust_jump(std::vector<four_tuple> & tuples){
	for (int i = 0; i < tuples.size(); ++i) {
		if (tuples[i].op == "j" || tuples[i].op == "jz" || tuples[i].op == "jnz"){
			tuples[i].result = std::to_string(std::stoi(tuples[i].result) + i + 1);
		}
	}
}

std::string inline Record::get_next_temp_var(const std::string & var1, const std::string & var2, std::vector<four_tuple> &buffer_tuple){
	std::string new_temp_name = "t" + std::to_string(++Record::temp_count);
	unsigned new_temp_type = has_real(var1, var2) ? REAL_NUM : INT_NUM;
	symbol_table[new_temp_name] = new_temp_type;
	if (new_temp_type == INT_NUM){
		buffer_tuple.push_back(generate_int(new_temp_name));
	}
	else if (new_temp_type == REAL_NUM){
		buffer_tuple.push_back(generate_double(new_temp_name));
	}
	return new_temp_name;
}

token inline Record::create_name_token1(SYM_TYPE type, const char * name){
	return create_strange_token(type, name);
}

bool inline Record::judge_strange_token(newNode *node_expr){
	if (node_expr->onetoken.type != STRANGE_TOKEN)
	{
		return false;
	}
	else
		return true;
}

bool inline Record::judge_null_token(newNode *node_expr){
	if (node_expr->onetoken.type != NULL_TOKEN)
	{
		return false;
	}
	else return true;
}

void inline Record::generate_declaration(std::vector<four_tuple> &buffer_tuple, newNode *declaration, std::string variable_name){
	if (declaration->onetoken == token{ TYPENAME, Type_INT }){
		buffer_tuple.push_back(Record::generate_int(variable_name));
	}
	else{
		buffer_tuple.push_back(Record::generate_double(variable_name));
	}
}

void inline Record::traverse_buffer(std::vector<four_tuple> & buffer_tuple, std::vector<four_tuple> & traversed_tuple){
	for (int i = 0; i < traversed_tuple.size(); i++){
		buffer_tuple.push_back(traversed_tuple[i]);
	}
}

void Record::optimization(std::vector<four_tuple> & buffer_tuple) {
	//优化1：消除多余的临时变量
	std::vector<unsigned> jmp_table;
	unsigned jmp_count = 0;
	std::set<std::string> optimization_table = {
		"+", "-", "*", "/",  ">",
		"<", ">=", "<=", "==", "!=", "&&", "||", 
	};
	std::vector<unsigned> del_table;
	for (int i = 0; i < buffer_tuple.size() - 1; ++i){
		if (optimization_table.find(buffer_tuple[i].op) != optimization_table.end()
			&& buffer_tuple[i].result[0] == 't' &&
			buffer_tuple[i + 1].op == "=" &&
			buffer_tuple[i].result == buffer_tuple[i + 1].arg1){
			del_table.push_back(i);
			++jmp_count;
		}
		jmp_table.push_back(jmp_count);
	}
	jmp_table.push_back(jmp_count);
	std::vector<four_tuple> result;
	unsigned now_pointer = 0;
	for (unsigned i = 0; i < buffer_tuple.size() - 1; ++i){
		if (now_pointer < del_table.size() && i == del_table[now_pointer]){
			result.push_back({ buffer_tuple[i].op, buffer_tuple[i].arg1,
				buffer_tuple[i].arg2, buffer_tuple[i + 1].result });
			++now_pointer;
			++i;
		}
		else {
			result.push_back(buffer_tuple[i]);
			if (result.back().op == "j" || result.back().op == "jz" || result.back().op == "jnz"){
				unsigned now_jmp = std::stoi(result[result.size() - 1].result) - 1;
				now_jmp -= jmp_table[now_jmp];
				result[result.size() - 1].result = std::to_string(now_jmp + 1);
			}
		}
	}
	result.push_back(buffer_tuple.back());

	//优化2：如果两个源操作数都是常数，直接计算，并改成赋值语句
	for (four_tuple & it : result){
		if (optimization_table.find(it.op) != optimization_table.end() &&
			check_if_number(it.arg1[0]) && check_if_number(it.arg2[0])){			
			if (symbol_table[it.result] == REAL_NUM){
				it.arg1 = std::to_string(cal(std::stof(it.arg1), std::stof(it.arg2), it.op));
			}
			else if (symbol_table[it.result] == INT_NUM){
				it.arg1 = std::to_string(cal(std::stoi(it.arg1), std::stoi(it.arg2), it.op));
			}
			it.op = "=";
			it.arg2 = "_";
		}
	}
	buffer_tuple = result;
}

std::string Record::translate_conpound(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	std::vector<four_tuple> buffer_ordered;
	for (int i = 0; i < node_exprs->children.size(); i++){
		translate_expr(buffer_ordered, node_exprs->children[i]);
	}
	traverse_buffer(buffer_tuple, buffer_ordered);
	return "";
}

//var_declaration有问题，还得接着遍历左边第一个节点
std::string Record::translate_var_declaration(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	newNode* declaration = node_exprs->children[0];
	for (int i = 1; i < node_exprs->children.size(); i++){
		newNode * now = node_exprs->children[i];
		newNode * action = node_exprs->children[i];
		if (now->onetoken.type == NULL_TOKEN) {
			continue;
		}
		while (now->onetoken.type != VARNAME){
			now = now->children[0];
		}

		generate_declaration(buffer_tuple, declaration, now->onetoken.value.var_name);
		translate_expr(buffer_tuple, action);
	}
	return "";
}

std::string Record::translate_if(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	/*
	condition True
	condition NULL
	condition True False
	condition NULL False
	condition NULL NULL
	condition True NULL
	*/
	newNode* cond = node_exprs->children[0];
	std::vector<four_tuple> buffer_cond;
	std::string str1 = translate_expr(buffer_cond, cond);

	if (cond->onetoken.type == NULL_TOKEN) {
		Record::output_failure("Statement if's condition shouldn't be empty!");
	}

	if (node_exprs->children.size() == 2){
		newNode* stmts = node_exprs->children[1];
		std::vector<four_tuple> buffer_stmts;

		if (stmts->onetoken.type == NULL_TOKEN){
			return "";
		}
		else{
			std::string str2 = translate_expr(buffer_stmts, stmts);
			//跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
			traverse_buffer(buffer_tuple, buffer_cond);
			four_tuple temp_tuple = four_tuple{ "jnz", str1, "_", std::to_string(2) };
			buffer_tuple.push_back(temp_tuple);
			//跳出if的指令
			int end_count = buffer_stmts.size() + 1;
			temp_tuple = Record::generate_no_cond_jump(end_count);
			buffer_tuple.push_back(temp_tuple);
			traverse_buffer(buffer_tuple, buffer_stmts);
			return "";
		}
	}
	else{
		newNode* true_branch = node_exprs->children[1];
		newNode* false_branch = node_exprs->children[2];
		std::vector<four_tuple> buffer_true;
		std::vector<four_tuple> buffer_false;
		//前后都空
		if (true_branch->onetoken.type == NULL_TOKEN && false_branch->onetoken.type == NULL_TOKEN){
			return "";
		}
		//前空后不空
		else if (true_branch->onetoken.type == NULL_TOKEN && !false_branch->onetoken.type == NULL_TOKEN){
			std::string str3 = translate_expr(buffer_false, false_branch);

			traverse_buffer(buffer_tuple, buffer_cond);

			//跳出if的指令（只有else能用，如果正确就跳出if）,否则顺序走false分支
			int end_count = buffer_false.size() + 1;
			four_tuple temp_tuple = four_tuple{ "jnz", str1, "_", std::to_string(end_count) };
			buffer_tuple.push_back(temp_tuple);

			traverse_buffer(buffer_tuple, buffer_false);
			return "";
		}
		//前不空后空
		else if (!true_branch->onetoken.type == NULL_TOKEN && false_branch->onetoken.type == NULL_TOKEN){
			std::string str2 = translate_expr(buffer_true, true_branch);
			//跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
			traverse_buffer(buffer_tuple, buffer_cond);
			four_tuple temp_tuple = four_tuple{ "jnz", str1, "_", std::to_string(2) };
			buffer_tuple.push_back(temp_tuple);
			//跳出if的指令
			int end_count = buffer_true.size() + 1;
			temp_tuple = Record::generate_no_cond_jump(end_count);
			buffer_tuple.push_back(temp_tuple);
			traverse_buffer(buffer_tuple, buffer_true);
			return "";

		}
		//前后都不空
		else{
			std::string str2 = translate_expr(buffer_true, true_branch);
			std::string str3 = translate_expr(buffer_false, false_branch);
			//输出一条判断语句有用么？这里不是判断语句，而可能是一串判断，比如a>b && b>c，这里不会输出跳转的
			//跳到true分支
			traverse_buffer(buffer_tuple, buffer_cond);
			four_tuple temp_tuple = four_tuple{ "jnz", str1, "_"
				, std::to_string(2) };
			buffer_tuple.push_back(temp_tuple);
			//跳到false分支
			int end_count = buffer_true.size() + 1;
			temp_tuple = Record::generate_no_cond_jump(end_count);
			buffer_tuple.push_back(temp_tuple);

			traverse_buffer(buffer_tuple, buffer_true);
			traverse_buffer(buffer_tuple, buffer_false);
			return "";
		}
	}
}

std::string Record::translate_do(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	newNode* cond = node_exprs->children[0];
	newNode* stmts = node_exprs->children[1];//stmt
	int project_base_address = Record::address;//在整个for开始的时候截取开始的地址，作为base_address
	//这个项目的基地址只用于之后的跳转语句定位，不作为输出语句的前序号

	if (cond->onetoken.type == NULL_TOKEN) {
		Record::output_failure("Statement do while's condition shouldn't be empty!");
	}


	std::vector<four_tuple> buffer_cond;
	std::string str1 = translate_expr(buffer_cond, cond);

	std::vector<four_tuple> buffer_stmts;
	std::string str2 = translate_expr(buffer_stmts, stmts);

	//先执行一遍语句
	traverse_buffer(buffer_tuple, buffer_stmts);

	traverse_buffer(buffer_tuple, buffer_cond);

	//跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
	four_tuple temp_tuple = four_tuple{ "jnz", str1, "_", std::to_string(-int(buffer_stmts.size()) - int(buffer_cond.size())) };
	buffer_tuple.push_back(temp_tuple);

	//跳出循环的指令
	int end_count = 1;
	//2条跳转指令，两条是循环和跳出循环的，还要加基地址（跳转指令专用）
	temp_tuple = Record::generate_no_cond_jump(end_count);
	buffer_tuple.push_back(temp_tuple);
	SYMBOL_TABLE optimization_table;
	//optimization(buffer_tuple, optimization_table);

	return "";
}

std::string Record::translate_while(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	newNode *cond = node_exprs->children[0];
	newNode *stmts = node_exprs->children[1];//stmt
	//int project_base_address = Record::address;//在整个for开始的时候截取开始的地址，作为base_address
	////这个项目的基地址只用于之后的跳转语句定位，不作为输出语句的前序号

	if (cond->onetoken.type == NULL_TOKEN) {
		Record::output_failure("Statement while's condition shouldn't be empty!");
	}
	std::vector<four_tuple> buffer_cond;
	std::string str1 = translate_expr(buffer_cond, cond);

	std::vector<four_tuple> buffer_stmts;
	std::string str2 = translate_expr(buffer_stmts, stmts);

	//先执行一遍语句
	traverse_buffer(buffer_tuple, buffer_cond);

	//跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
	four_tuple temp_tuple = four_tuple{ "jnz", str1, "_", std::to_string(2) };
	buffer_tuple.push_back(temp_tuple);
	//跳出循环的指令
	int end_count = buffer_stmts.size() + 2;
	//2条跳转指令，两条是循环和跳出循环的，还要加基地址（跳转指令专用）
	temp_tuple = Record::generate_no_cond_jump(end_count);
	buffer_tuple.push_back(temp_tuple);

	traverse_buffer(buffer_tuple, buffer_stmts);
	//跳回循环判断
	temp_tuple = Record::generate_no_cond_jump(-int(buffer_cond.size()) - int(buffer_stmts.size()) - 2);
	buffer_tuple.push_back(temp_tuple);
	SYMBOL_TABLE optimization_table;
	//optimization(buffer_tuple, optimization_table);

	return "";
}


//for也必须有buffer，而且不能直接写，因为for可能是嵌套在别的for里面的
std::string Record::translate_for(std::vector<four_tuple> &buffer_tuple, newNode *node_for){
	newNode *e1 = node_for->children[0];//for1
	newNode *e2 = node_for->children[1];//for2
	newNode *e3 = node_for->children[2];//for3
	newNode *stmts = node_for->children[3];//stmt

	int project_base_address = Record::address;//在整个for开始的时候截取开始的地址，作为base_address
	//这个项目的基地址只用于之后的跳转语句定位，不作为输出语句的前序号

	std::vector<four_tuple> buffer_e1;
	std::string str1 = translate_expr(buffer_e1, e1);

	std::vector<four_tuple> buffer_e2;
	std::string str2 = translate_expr(buffer_e2, e2);

	std::vector<four_tuple> buffer_e3;
	std::string str3 = translate_expr(buffer_e3, e3);

	std::vector<four_tuple> buffer_stmts;
	std::string str4 = translate_expr(buffer_stmts, stmts);

	traverse_buffer(buffer_tuple, buffer_e1);

	traverse_buffer(buffer_tuple, buffer_e2);

	//跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
	four_tuple temp_tuple = four_tuple{ "jnz", str2, "_"
		, std::to_string(buffer_e3.size() + 3) };//跳到stmt（stmt之前还有一条跳回）
	buffer_tuple.push_back(temp_tuple);

	//跳出循环的指令
	int end_count = buffer_e3.size() + buffer_stmts.size() + 3;
	//4条跳转指令，两条是e2的，两条是后面循环的，还要加基地址（跳转指令专用）

	temp_tuple = Record::generate_no_cond_jump(end_count);
	buffer_tuple.push_back(temp_tuple);

	traverse_buffer(buffer_tuple, buffer_e3);

	temp_tuple = Record::generate_no_cond_jump(-int(buffer_e3.size()) - int(buffer_e2.size()) - 2);//不用-1，因为正好跳回e2的第一条指令
	buffer_tuple.push_back(temp_tuple);

	traverse_buffer(buffer_tuple, buffer_stmts);

	temp_tuple = Record::generate_no_cond_jump(-1 - int(buffer_stmts.size()) - int(buffer_e3.size()));//跳回e3第一条指令
	buffer_tuple.push_back(temp_tuple);

	SYMBOL_TABLE optimization_table;
	//optimization(buffer_tuple, optimization_table);

	return "";
}

std::string Record::judge_type_to_string(newNode* node_exprs)
{
	if (node_exprs->onetoken.type == VARNAME) {
		return std::string(node_exprs->onetoken.value.var_name);
	}
	else if (node_exprs->onetoken.type == INT_NUM) {
		return std::to_string(node_exprs->onetoken.value.int_value);
	}
	else if (node_exprs->onetoken.type == REAL_NUM) {
		return std::to_string(node_exprs->onetoken.value.real_value);
	}
}

std::string Record::translate_scanf(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	/*for (int i = 0; i < node_exprs->children.size(); i++) {
	newNode *input = node_exprs->children[i];
	if (input->onetoken.type == NULL_TOKEN) {
	continue;
	}
	four_tuple temp_tuple = Record::generate_scanf(judge_type_to_string(input));
	buffer_tuple.push_back(temp_tuple);
	}
	*/
	if (node_exprs->onetoken.type == NULL_TOKEN) {
		return "";
	}
	for (int i = 0; i < node_exprs->children.size(); i++) {
		newNode *input = node_exprs->children[i];
		if (input->onetoken.type == NULL_TOKEN) {
			continue;
		}
		if (input->onetoken == create_name_token1(STRANGE_TOKEN, "conpound")) {
			for (int i = 0; i < input->children.size(); i++) {
				translate_scanf(buffer_tuple, input->children[i]);
			}
			return "";
			//递归找子节点
		}
	}
	//如果遍历到底部了，就生成四元式
	if (node_exprs->onetoken != create_name_token1(STRANGE_TOKEN, "conpound")
		&& node_exprs->onetoken != create_name_token1(STRANGE_TOKEN, "scanf"))
	{
		four_tuple temp_tuple = Record::generate_scanf(judge_type_to_string(node_exprs));
		buffer_tuple.push_back(temp_tuple);
	}
	return "";
}

std::string Record::translate_printf(std::vector<four_tuple> &buffer_tuple, newNode *node_exprs){
	if (node_exprs->onetoken.type == NULL_TOKEN) {
		return "";
	}
	for (int i = 0; i < node_exprs->children.size(); i++) {
		newNode *output = node_exprs->children[i];
		if (output->onetoken.type == NULL_TOKEN) {
			continue;
		}
		if (output->onetoken == create_name_token1(STRANGE_TOKEN, "conpound")) {
			for (int i = 0; i < output->children.size(); i++) {
				translate_printf(buffer_tuple, output->children[i]);
			}
			return "";
			//递归找子节点
		}
	}
	//如果遍历到底部了，就生成四元式
	if (node_exprs->onetoken != create_name_token1(STRANGE_TOKEN, "conpound")
		&& node_exprs->onetoken != create_name_token1(STRANGE_TOKEN, "printf"))
	{
		//std::cout << "-------------------" << std::endl;
		//std::cout << node_exprs->onetoken << std::endl;
		four_tuple temp_tuple = Record::generate_printf(judge_type_to_string(node_exprs));
		buffer_tuple.push_back(temp_tuple);
	}
	return "";
}

token Record::get_left_token(newNode * root) {
	if (root->children.size() == 0) {
		return root->onetoken;
	}
	else if (root->children[0]->onetoken.type != STRANGE_TOKEN) {
		return root->children[0]->onetoken;
	}
	else {
		return get_left_token(root->children[0]);
	}
}

void Record::add_to_vars(newNode * root){
	if (root->onetoken.type == VARNAME){
		STR_NAME p = new char[strlen(root->onetoken.value.var_name) + 2];
		p[0] = '_';
		p[1] = 0;
		strcat(p, root->onetoken.value.var_name);
		delete []root->onetoken.value.var_name;
		root->onetoken.value.var_name = p;
	}
	for (int i = 0; i < root->children.size(); ++i){
		add_to_vars(root->children[i]);
	}
}

bool Record::type_check_two_variable(newNode* node1, newNode* node2)
{
	int type1 = symbol_table[node1->onetoken.value.var_name];
	int type2 = symbol_table[node2->onetoken.value.var_name];
	switch (type1) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		if (type2 == Type_Pointer)
			return false;
		else
			return true;
	case 5:
		if (type2 != Type_Pointer)
			return false;
		else
			return true;
	}
}

std::string Record::create_temp_variable_binary(std::vector<four_tuple> &buffer_tuple, newNode *node_expr, std::string opera){
	std::string temp_variable1 = "";
	std::string temp_variable2 = "";

	//        用strange_token单纯判断是叶子还是不是，不对！！！
	if (judge_strange_token(node_expr->children[0])){
		temp_variable1 = translate_expr(buffer_tuple, node_expr->children[0]);
	}
	if (judge_strange_token(node_expr->children[1])){
		temp_variable2 = translate_expr(buffer_tuple, node_expr->children[1]);
	}

	temp_variable1 = temp_variable1[0] ? temp_variable1 : judge_type_to_string(node_expr->children[0]);
	temp_variable2 = temp_variable2[0] ? temp_variable2 : judge_type_to_string(node_expr->children[1]);
	buffer_tuple.push_back(Record::generate_binary_operator(opera, temp_variable1, temp_variable2, buffer_tuple));

	if (opera != "=") 
		return "t" + std::to_string(Record::temp_count);
	else if (temp_variable1 != "")
		return temp_variable1;
	else
		return judge_type_to_string(node_expr->children[0]);
}

std::string Record::translate_expr(std::vector<four_tuple> &buffer_tuple, newNode *node_expr) {
	//二元运算符部分
	//std::cout << "-------------------" << std::endl;
	//std::cout << node_expr->onetoken << std::endl;
	if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "+")) {

		return create_temp_variable_binary(buffer_tuple, node_expr, "+");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "-")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "-");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "*")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "*");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "/")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "/");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, ">")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, ">");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "<")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "<");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "<=")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "<=");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, ">=")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, ">=");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "==")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "==");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "!=")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "!=");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "&&")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "&&");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "||")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "||");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "%")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "%");
	}
	//赋值的四元式与其他不同
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "=")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "=");
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "^")) {
		return create_temp_variable_binary(buffer_tuple, node_expr, "^");
		//一元运算符部分
	}
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "!")) {
		return create_temp_variable_unary(buffer_tuple, node_expr, "!");
	}
	//语句部分没什么返回值
	//if语句部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "if")) {
		translate_if(buffer_tuple, node_expr);
		return "";
	}
	//for语句部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "for")) {
		translate_for(buffer_tuple, node_expr);
		return "";
	}
	//while语句部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "while")) {
		translate_while(buffer_tuple, node_expr);
		return "";
	}
	//do语句部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "do")) {
		translate_do(buffer_tuple, node_expr);
		return "";
	}
	//var_declaration部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "var_declaration")) {
		translate_var_declaration(buffer_tuple, node_expr);
		return "";
	}
	//conpound 部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "conpound")) {
		translate_conpound(buffer_tuple, node_expr);
		return "";
	}
	//scanf部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "scanf")) {
		translate_scanf(buffer_tuple, node_expr);
		return "";
	}
	//printf部分
	else if (node_expr->onetoken == create_name_token1(STRANGE_TOKEN, "printf")) {
		translate_printf(buffer_tuple, node_expr);
		return "";
	}
	//为空直接返回
	else if (node_expr->onetoken.type == NULL_TOKEN) {
		return "";
	}
	else {
		return "";
	}
}

#endif