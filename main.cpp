#include "lex.h"

int main()
{
	Lex lex = CreateLex("1.txt");
	token tk = ERR_TOKEN;
	while (tk.type != EOF_TOKEN.type)
	{
		tk = GetNextToken(&lex);
		printf("%d %d\n", tk.type, tk.value.int_value);
	}	
	system("pause");
	return 0;
}
