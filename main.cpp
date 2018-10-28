#include "lex.h"

int main()
{
	Lex lex("1.txt");
	token tk = ERR_TOKEN;
	while (tk.type != EOF_TOKEN.type)
	{
		tk = lex.get_token();
		if (tk.type == VARNAME)
			printf("变量 名称地址是 %p\n", tk.value.var_name);
		else if (tk.type == INT_NUM)
			printf("整数 %d\n", tk.value.int_value);
		else if (tk.type == REAL_NUM)
			printf("浮点数 %lf\n", tk.value.real_value);
		else if (tk.type == STRING)
			printf("字符串 首字符地址是 %p\n", tk.value.var_name);
		else
			printf("类型：%d %d\n", tk.type, tk.value.sym_name);
	}

	system("pause");
	return 0;
}
