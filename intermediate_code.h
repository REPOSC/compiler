#include <fstream>
#include<iostream>
#include <assert.h>
#include"token.h"
#include"yacc.h"
#include"type.h"

struct four_tuple{
	std::string arg1;
	std::string arg2;
	std::string result;
	std::string op;
	four_tuple(std::string op,std::string arg1,std::string arg2,std::string result){
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
public:
	static int address;
	static int address_output;
	static int temp_count;//临时变量计数
	static four_tuple generate_no_cond_jump(int address);
	static four_tuple generate_cond_jump(std::string former,std::string latter,int address);
	static four_tuple generate_binary_operator(std::string my_binary_operator,std::string arg1,std::string arg2);
    static four_tuple generate_unary_operator(std::string my_binary_operator,std::string arg1);
	static four_tuple generate_scanf(std::string input);
	static four_tuple generate_printf(std::string output);
	static four_tuple generate_int(std::string variable_name);
	static four_tuple generate_double(std::string variable_name);
	static void output_my_four_tuple(four_tuple &data);
	static void output_my_four_tuple1(four_tuple &data);
	static void output_failure(std::string error);
};

int Record::address = 0;
int Record::address_output = 0;

int Record::temp_count = -1;

four_tuple Record::generate_no_cond_jump(int address){
	return four_tuple{"j","_","_",std::to_string(address)};
	//动态分配较好，否则这个值是拷贝传回,浪费
}

four_tuple Record::generate_cond_jump(std::string former,std::string latter,int address){
	//如果这个条件是1或者0或者varname这样的变量，就判断一下值，然后转为j指令（无条件跳转）
	return four_tuple{"jnz",former,latter,std::to_string(address)};
}

four_tuple Record::generate_binary_operator(std::string my_binary_operator,std::string arg1,std::string arg2){
    if(my_binary_operator != "=")
        return four_tuple{my_binary_operator,arg1,arg2,"t"+std::to_string(++Record::temp_count)};
    else
		return four_tuple{ "=",arg2,"_",arg1 };
}

four_tuple Record::generate_unary_operator(std::string my_unary_operator,std::string arg1){
	return four_tuple{my_unary_operator,arg1,"_","t"+std::to_string(++Record::temp_count)};
}

//(s,a,_,_)
four_tuple Record::generate_scanf(std::string input){
	return four_tuple{"s",input,"_","_"};
}
//(p,a,_,_)
four_tuple Record::generate_printf(std::string output){
	return four_tuple{"p",output,"_","_"};
}
//(i,i,_,2)
four_tuple Record::generate_int(std::string variable_name){
	return four_tuple{"i",variable_name,"_","_"};
}
//(d,b,_,_)
four_tuple Record::generate_double(std::string variable_name){
	return four_tuple{"d",variable_name,"_","_"};
}


void Record::output_my_four_tuple1(four_tuple &data)
{
	std::ofstream outfile;
	outfile.open("output_compile.txt", std::ios::app);
	// 再次向文件写入用户输入的数据
	outfile << "(" <<
		data.op.c_str() << "," << data.arg1.c_str() << "," << data.arg2.c_str()
		<< "," << data.result.c_str() << ")" << std::endl;
	// 关闭打开的文件
	outfile.close();
}

void Record::output_my_four_tuple(four_tuple &data)
{
	std::ofstream outfile;
	outfile.open("output.txt", std::ios::app);
	// 再次向文件写入用户输入的数据
	outfile << Record::address_output++ << "," <<
		data.op.c_str() << "," << data.arg1.c_str() << "," << data.arg2.c_str()
		<< "," << data.result.c_str() << std::endl;
	// 关闭打开的文件
	outfile.close();
}

void Record::output_failure(std::string error)
{
	std::ofstream outfile;
	outfile.open("error.txt",std::ios::app);
	// 再次向文件写入用户输入的数据
	outfile << error<<std::endl;
	// 关闭打开的文件
	outfile.close();
}



token create_name_token1(SYM_TYPE type, const char * name){
	return create_strange_token(type,name);
}

bool judge_strange_token(newNode *node_expr){
    if(node_expr->onetoken.type != STRANGE_TOKEN)
    {
        return false;
    }
    else
        return true;
}

bool judge_null_token(newNode *node_expr){
    if(node_expr->onetoken.type != NULL_TOKEN)
    {
        return false;
    }
    else return true;
}

void traverse_buffer(std::vector<four_tuple> & buffer_tuple, std::vector<four_tuple> & traversed_tuple){
    for(int i = 0;i < traversed_tuple.size();i++){
        buffer_tuple.push_back(traversed_tuple[i]);
    }
}

std::string translate_expr(std::vector<four_tuple> &buffer_tuple, newNode *node_expr);
void optimization(std::vector<four_tuple> & buffer_tuple, SYMBOL_TABLE optimization_table) {
	for (int i = 0; i < buffer_tuple.size(); i++) {
		if (buffer_tuple[i].op == "=" && buffer_tuple[i].result[0] != 't'
			&& !((buffer_tuple[i].result[0] >= 48) && (buffer_tuple[i].result[0] <= 57))
			&& ((buffer_tuple[i].arg1[0] >= 48) && (buffer_tuple[i].arg1[0] <= 57))) {

			bool not_exist = true;
			for (std::unordered_map<char*, int>::iterator iter = optimization_table.begin(); iter != optimization_table.end(); iter++)
			{
				if (strcmp(iter->first, buffer_tuple[i].result.c_str()) == 0)
				{
					iter->second = std::stoi(buffer_tuple[i].arg1);
					not_exist = false;
				}
			}
			if (not_exist)
				optimization_table.insert({ (char*)(buffer_tuple[i].result).c_str(), std::stoi(buffer_tuple[i].arg1) });
		}
	}
	//确保i出现在右值之前没有再定义，且没有出现在赋值语句的左值，给右值为i的地方进行优化
	for (std::unordered_map<char*, int>::iterator iter = optimization_table.begin(); iter != optimization_table.end(); iter++)
	{
		for (int i = 0; i < buffer_tuple.size(); i++) {
			if (buffer_tuple[i].arg2 == std::string(iter->first)) {
				bool re_defined = false;
				int count = 0;
				for (int j = 0; j < buffer_tuple.size(); j++) {
					if (buffer_tuple[j].op == "=" && buffer_tuple[j].result == iter->first) {
						re_defined = true;
						count++;
					}
					if ((buffer_tuple[j].op == "i" || buffer_tuple[j].op == "j") && buffer_tuple[j].arg1 == iter->first) {
						re_defined = true;
						count++;
					}
				}
				if (count == 1) {
					buffer_tuple[i].arg2 = std::to_string(iter->second);
				}
			}
			else if (buffer_tuple[i].arg1 == std::string(iter->first)) {
				for (int j = 0; j < buffer_tuple.size(); j++) {
					bool re_defined = false;
					int count = 0;
					if (buffer_tuple[j].op == "=" && buffer_tuple[j].result == iter->first) {
						re_defined = true;
						count++;
					}
					if ((buffer_tuple[j].op == "i" || buffer_tuple[j].op == "j") && buffer_tuple[j].arg1 == iter->first) {
						re_defined = true;
						count++;
					}
					if (count == 1 && buffer_tuple[i].op != "i" && buffer_tuple[i].op != "d" && buffer_tuple[i].arg1 != "i"
						&& buffer_tuple[i].arg1 != "x"&& buffer_tuple[i].arg1 != "j"
						&& buffer_tuple[i].arg1 != "n") {
						buffer_tuple[i].arg1 = std::to_string(iter->second);
					}

				}

			}

		}
	}
}

std::string translate_conpound(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
    std::vector<four_tuple> buffer_ordered;
    for(int i = 0;i < node_exprs->children.size();i++){
        translate_expr(buffer_ordered,node_exprs->children[i]);
    }
    traverse_buffer(buffer_tuple,buffer_ordered);
    return "";
}

//SYMBOL_TABLE my_symbol_table;

void generate_declaration(std::vector<four_tuple> &buffer_tuple,newNode *declaration,std::string variable_name){
    if(declaration->onetoken == token{TYPENAME, Type_INT}){
		buffer_tuple.push_back(Record::generate_int(variable_name));
		////my_symbol_table[variable_name] = Type_INT;
		//my_symbol_table.insert({ variable_name.c_str(), Type_INT });
    }
    else{
		buffer_tuple.push_back(Record::generate_double(variable_name));
		//my_symbol_table[variable_name.c_str()] = Type_DOUBLE;
		//yacc_symbol_table.insert({ symboltoken.value.var_name, typetoken.value.sym_name });
	}
}



//var_declaration有问题，还得接着遍历左边第一个节点
std::string translate_var_declaration(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
    newNode* declaration = node_exprs->children[0];
    for(int i = 1; i < node_exprs->children.size();i++){
        newNode * now = node_exprs->children[i];
        newNode * action = node_exprs->children[i];
		if (now->onetoken.type == NULL_TOKEN) {
			continue;
		}
        while(now->onetoken.type != VARNAME){
			now = now->children[0];
        }

        generate_declaration(buffer_tuple,declaration,now->onetoken.value.var_name);
		translate_expr(buffer_tuple,action);
    }
	return "";
}

std::string translate_if(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
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
	std::string str1 = translate_expr(buffer_cond,cond);

	if (cond->onetoken.type == NULL_TOKEN) {
		Record::output_failure("Statement if's condition shouldn't be empty!");
	}

    if(node_exprs->children.size() == 2){
            newNode* stmts = node_exprs->children[1];
            std::vector<four_tuple> buffer_stmts;

            if(stmts->onetoken.type = NULL_TOKEN){
                return "";
            }
            else{
                std::string str2 = translate_expr(buffer_stmts,stmts);
                //跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
                traverse_buffer(buffer_tuple,buffer_cond);
                four_tuple temp_tuple = four_tuple{"jnz",str1,"_",std::to_string(2)};
                buffer_tuple.push_back(temp_tuple);
                //跳出if的指令
                int end_count = buffer_stmts.size()+1;
                temp_tuple = Record::generate_no_cond_jump(end_count);
                buffer_tuple.push_back(temp_tuple);
                traverse_buffer(buffer_tuple,buffer_stmts);
                return "";
            }
    }
    else{
        newNode* true_branch = node_exprs->children[1];
        newNode* false_branch = node_exprs->children[2];
        std::vector<four_tuple> buffer_true;
        std::vector<four_tuple> buffer_false;
        //前后都空
        if(true_branch->onetoken.type == NULL_TOKEN && false_branch->onetoken.type == NULL_TOKEN){
            return "";
        }
        //前空后不空
        else if(true_branch->onetoken.type == NULL_TOKEN && !false_branch->onetoken.type == NULL_TOKEN){
            std::string str3 = translate_expr(buffer_false,false_branch);

            traverse_buffer(buffer_tuple,buffer_cond);

            //跳出if的指令（只有else能用，如果正确就跳出if）,否则顺序走false分支
            int end_count = buffer_false.size()+1;
            four_tuple temp_tuple = four_tuple{"jnz",str1,"_",std::to_string(end_count)};
            buffer_tuple.push_back(temp_tuple);

            traverse_buffer(buffer_tuple,buffer_false);
            return "";
        }
        //前不空后空
        else if(!true_branch->onetoken.type == NULL_TOKEN && false_branch->onetoken.type == NULL_TOKEN){
            std::string str2 = translate_expr(buffer_true,true_branch);
            //跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
            traverse_buffer(buffer_tuple,buffer_cond);
            four_tuple temp_tuple = four_tuple{"jnz",str1,"_",std::to_string(2)};
            buffer_tuple.push_back(temp_tuple);
            //跳出if的指令
            int end_count = buffer_true.size()+1;
			temp_tuple = Record:: generate_no_cond_jump(end_count);
            buffer_tuple.push_back(temp_tuple);
            traverse_buffer(buffer_tuple,buffer_true);
            return "";

        }
        //前后都不空
        else{
            std::string str2 = translate_expr(buffer_true,true_branch);
            std::string str3 = translate_expr(buffer_false,false_branch);
            //输出一条判断语句有用么？这里不是判断语句，而可能是一串判断，比如a>b && b>c，这里不会输出跳转的
            //跳到true分支
            traverse_buffer(buffer_tuple,buffer_cond);
            four_tuple temp_tuple = four_tuple{"jnz",str1,"_"
            ,std::to_string(2)};
            buffer_tuple.push_back(temp_tuple);
            //跳到false分支
            int end_count = buffer_true.size()+1;
            temp_tuple = Record::generate_no_cond_jump(end_count);
            buffer_tuple.push_back(temp_tuple);

            traverse_buffer(buffer_tuple,buffer_true);
            traverse_buffer(buffer_tuple,buffer_false);
            return "";
        }

    }
}

std::string translate_do(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
    newNode* cond = node_exprs->children[0];
	newNode* stmts = node_exprs->children[1];//stmt
    int project_base_address = Record::address;//在整个for开始的时候截取开始的地址，作为base_address
    //这个项目的基地址只用于之后的跳转语句定位，不作为输出语句的前序号

	if (cond->onetoken.type == NULL_TOKEN) {
		Record::output_failure("Statement do while's condition shouldn't be empty!");
	}


	std::vector<four_tuple> buffer_cond;
	std::string str1 = translate_expr(buffer_cond,cond);

	std::vector<four_tuple> buffer_stmts;
	std::string str2 = translate_expr(buffer_stmts,stmts);

	//先执行一遍语句
    traverse_buffer(buffer_tuple,buffer_stmts);

    traverse_buffer(buffer_tuple,buffer_cond);

    //跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
    four_tuple temp_tuple = four_tuple{"jnz",str1,"_",std::to_string(-int(buffer_stmts.size())-int(buffer_cond.size()))};
    buffer_tuple.push_back(temp_tuple);

    //跳出循环的指令
    int end_count = 1;
    //2条跳转指令，两条是循环和跳出循环的，还要加基地址（跳转指令专用）
    temp_tuple = Record::generate_no_cond_jump(end_count);
    buffer_tuple.push_back(temp_tuple);
	SYMBOL_TABLE optimization_table;
	optimization(buffer_tuple, optimization_table);

    return "";
}

std::string translate_while(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
    newNode *cond = node_exprs->children[0];
	newNode *stmts = node_exprs->children[1];//stmt
    //int project_base_address = Record::address;//在整个for开始的时候截取开始的地址，作为base_address
    ////这个项目的基地址只用于之后的跳转语句定位，不作为输出语句的前序号
	
	if (cond->onetoken.type == NULL_TOKEN) {
		Record::output_failure("Statement while's condition shouldn't be empty!");
	}
	std::vector<four_tuple> buffer_cond;
	std::string str1 = translate_expr(buffer_cond,cond);

	std::vector<four_tuple> buffer_stmts;
	std::string str2 = translate_expr(buffer_stmts,stmts);

	//先执行一遍语句
    traverse_buffer(buffer_tuple,buffer_cond);

    //跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
    four_tuple temp_tuple = four_tuple{"jnz",str1,"_",std::to_string(2)};
    buffer_tuple.push_back(temp_tuple);
    //跳出循环的指令
    int end_count = buffer_stmts.size()+2;
    //2条跳转指令，两条是循环和跳出循环的，还要加基地址（跳转指令专用）
    temp_tuple = Record::generate_no_cond_jump(end_count);
    buffer_tuple.push_back(temp_tuple);

    traverse_buffer(buffer_tuple,buffer_stmts);
	//跳回循环判断
	temp_tuple = Record::generate_no_cond_jump(-int(buffer_cond.size())-int(buffer_stmts.size())-2);
	buffer_tuple.push_back(temp_tuple);
	SYMBOL_TABLE optimization_table;
	optimization(buffer_tuple, optimization_table);


    return "";
}


//for也必须有buffer，而且不能直接写，因为for可能是嵌套在别的for里面的

std::string translate_for(std::vector<four_tuple> &buffer_tuple,newNode *node_for){
	newNode *e1 = node_for->children[0];//for1
	newNode *e2 = node_for->children[1];//for2
	newNode *e3 = node_for->children[2];//for3
	newNode *stmts = node_for->children[3];//stmt
	
    int project_base_address = Record::address;//在整个for开始的时候截取开始的地址，作为base_address
    //这个项目的基地址只用于之后的跳转语句定位，不作为输出语句的前序号

	std::vector<four_tuple> buffer_e1;
	std::string str1 = translate_expr(buffer_e1,e1);

	std::vector<four_tuple> buffer_e2;
	std::string str2 = translate_expr(buffer_e2,e2);

	std::vector<four_tuple> buffer_e3;
	std::string str3 = translate_expr(buffer_e3,e3);

	std::vector<four_tuple> buffer_stmts;
    std::string str4 = translate_expr(buffer_stmts,stmts);

    traverse_buffer(buffer_tuple,buffer_e1);

    traverse_buffer(buffer_tuple,buffer_e2);

    //跳到执行语句的指令,str2是我取出的e2运行的总结果的临时变量
    four_tuple temp_tuple = four_tuple{"jnz",str2,"_"
    ,std::to_string(buffer_e3.size()+3)};//跳到stmt（stmt之前还有一条跳回）
    buffer_tuple.push_back(temp_tuple);

    //跳出循环的指令
    int end_count = buffer_e3.size()+buffer_stmts.size()+3;
    //4条跳转指令，两条是e2的，两条是后面循环的，还要加基地址（跳转指令专用）

    temp_tuple = Record::generate_no_cond_jump(end_count);
    buffer_tuple.push_back(temp_tuple);

    traverse_buffer(buffer_tuple,buffer_e3);

    temp_tuple = Record::generate_no_cond_jump(-int(buffer_e3.size())- int(buffer_e2.size())-2);//不用-1，因为正好跳回e2的第一条指令
    buffer_tuple.push_back(temp_tuple);

    traverse_buffer(buffer_tuple,buffer_stmts);

    temp_tuple = Record::generate_no_cond_jump(-1-int(buffer_stmts.size())- int(buffer_e3.size()));//跳回e3第一条指令
    buffer_tuple.push_back(temp_tuple);

	SYMBOL_TABLE optimization_table;
	optimization(buffer_tuple, optimization_table);


	return "";
}

std::string judge_type_to_string(newNode* node_exprs)
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

std::string translate_scanf(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
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

std::string translate_printf(std::vector<four_tuple> &buffer_tuple,newNode *node_exprs){
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

#define Type_CHAR 0
#define Type_DOUBLE 1
#define Type_FLOAT 2
#define Type_INT 3
#define Type_SHORT 4
#define Type_Pointer 5
SYMBOL_TABLE yacc_symbol_table;

token get_left_token(newNode * root) {
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

void get_symbol_table(newNode * root) {
	if (!root || root->onetoken.type != STRANGE_TOKEN) {
		return;
	}
	else {
		for (auto tempnode : root->children) {
			if (tempnode->onetoken.type == STRANGE_TOKEN) {
				if (std::string(tempnode->onetoken.value.strange_name) == "var_declaration") {
					token typetoken = tempnode->children[0]->onetoken;
					for (int tempindex = 1; tempindex < tempnode->children.size(); tempindex++) {
						token symboltoken = get_left_token(tempnode->children[tempindex]);
						if (symboltoken.type != NULL_TOKEN) {
							bool exist = true;
							for (std::unordered_map<char*, int>::iterator iter = yacc_symbol_table.begin(); iter != yacc_symbol_table.end(); iter++)
							{
								if (strcmp(iter->first, symboltoken.value.var_name) == 0)
								{
									iter->second = typetoken.value.sym_name;
									exist = false;
								}
							}
							if(exist)
								yacc_symbol_table.insert({ symboltoken.value.var_name,typetoken.value.sym_name });
						}
					}
				}
				else {
					get_symbol_table(tempnode);
				}
			}
		}
	}
}
SYMBOL_TABLE read_symbol_table() {
	std::cout << std::endl;
	return yacc_symbol_table;
}



bool type_check_two_variable(newNode* node1, newNode* node2)
{
	SYMBOL_TABLE symbol_table = read_symbol_table();
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

void generate_temporary_variable_declaration(std::vector<four_tuple> &buffer_tuple, newNode *declaration, std::string variable_name) {
	if (declaration->onetoken.type == INT_NUM) {
		buffer_tuple.push_back(Record::generate_int(variable_name));
	}
	else if (declaration->onetoken.type == REAL_NUM) {
		buffer_tuple.push_back(Record::generate_double(variable_name));
	}
	else if (declaration->onetoken.type == VARNAME) {
		std::string name = declaration->onetoken.value.var_name;
		for (std::unordered_map<char*, int>::iterator iter = yacc_symbol_table.begin(); iter != yacc_symbol_table.end(); iter++)
		{
			if (strcmp(iter->first, declaration->onetoken.value.var_name) == 0)
			{
				int type = iter->second;
				if (type == INT_NUM) {
					buffer_tuple.push_back(Record::generate_int(variable_name));
				}
				else if (type == REAL_NUM) {
					buffer_tuple.push_back(Record::generate_double(variable_name));
				}
			}
		}

	}
	else
		Record::output_failure("Type not match");
}



std::string create_temp_variable_binary(std::vector<four_tuple> &buffer_tuple, newNode *node_expr,std::string opera){
	std::string temp_variable1 = "";
	std::string temp_variable2 = "";

//        用strange_token单纯判断是叶子还是不是，不对！！！
        if(judge_strange_token(node_expr->children[0])){
             temp_variable1 = translate_expr(buffer_tuple,node_expr->children[0]);
        }
        if(judge_strange_token(node_expr->children[1])){
             temp_variable2 = translate_expr(buffer_tuple,node_expr->children[1]);
        }
        if(temp_variable1 != "" && temp_variable2 != ""){
		//左右都是临时变量，就会找不到类型，那么就把临时变量存到符号表里
		//generate_temporary_variable_declaration(buffer_tuple, , "t" + std::to_string(Record::temp_count));
        four_tuple temp = Record::generate_binary_operator(opera,temp_variable1,temp_variable2);
        buffer_tuple.push_back(temp);
        }
        else if(temp_variable1 != "" && temp_variable2 == ""){
		//generate_temporary_variable_declaration(buffer_tuple,node_expr->children[1], "t" + std::to_string(Record::temp_count + 1));
        four_tuple temp = Record::generate_binary_operator(opera,temp_variable1,judge_type_to_string(node_expr->children[1]));
        buffer_tuple.push_back(temp);
        }
        else if(temp_variable1 == "" && temp_variable2 != ""){
		//generate_temporary_variable_declaration(buffer_tuple,node_expr->children[0],"t" + std::to_string(Record::temp_count + 1));
        four_tuple temp = Record::generate_binary_operator(opera,judge_type_to_string(node_expr->children[0]),temp_variable2);
        buffer_tuple.push_back(temp);
        }
        else{
			if (!type_check_two_variable(node_expr->children[0], node_expr->children[1]))
				Record::output_failure("Type does not match");
		//generate_temporary_variable_declaration(buffer_tuple,node_expr->children[0],"t" + std::to_string(Record::temp_count + 1));
        four_tuple temp = Record::generate_binary_operator(opera,judge_type_to_string(node_expr->children[0]),judge_type_to_string(node_expr->children[1]));
        buffer_tuple.push_back(temp);
        }
		if (opera != "=") {
			return "t" + std::to_string(Record::temp_count);
		}
        else if(temp_variable1 != "")
            return temp_variable1;
        else
            return judge_type_to_string(node_expr->children[0]);
}


std::string create_temp_variable_unary(std::vector<four_tuple> &buffer_tuple, newNode *node_expr,std::string opera){
	std::string temp_variable1 = "";
        if(judge_strange_token(node_expr->children[0])){
             temp_variable1 = translate_expr(buffer_tuple,node_expr->children[0]);
        }
        if(temp_variable1 != ""){
        four_tuple temp = Record::generate_unary_operator(opera,temp_variable1);
        buffer_tuple.push_back(temp);
        }
        else{
        four_tuple temp = Record::generate_unary_operator(opera,judge_type_to_string(node_expr->children[0]));
        buffer_tuple.push_back(temp);
        }
        return "t"+std::to_string(Record::temp_count);
}


std::string translate_expr(std::vector<four_tuple> &buffer_tuple, newNode *node_expr) {
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


