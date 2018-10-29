#include "lex.h"

int main(int argc, char ** argv)
{
	if (argc <= 1)
	{
		fprintf(stderr, "Fetal: No input file.\n");
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
		while (tk.type != EOF_TOKEN.type)
		{
			tk = lex.get_token();
			lex.Symbol_Token();
			switch(tk.type)
			{
				case VARNAME:    printf("变量名 地址是 %p\n", tk.value.var_name); break;
				case INT_NUM:    printf("整数 %d\n", tk.value.int_value); break;
				case REAL_NUM:   printf("浮点数 %lf\n", tk.value.real_value); break;
				case STRING:     printf("字符串 首字符地址是 %p\n", tk.value.str_name); break;
				case DELIMITER:  printf("分割符 其类型编号是 %d\n", tk.value.sym_name); break;
				case CONTROLLER: printf("控制字符 其类型编号是 %d\n", tk.value.sym_name); break;
				case OPERATOR:   printf("运算符 其类型编号是 %d\n", tk.value.sym_name); break;
				case ERR_TYPE:   printf("无效字符!\n"); break;
				case EOF_TYPE:   printf("文件结束。\n"); break;
			}
		}
		return 0;
	}
	catch (File_Error)
	{
		fprintf(stderr, "Fetal: Cannot open file %s.\n", argv[1]);
		exit(-1);
	}
}
