#include "InputStream.h"

int main()
{
	InputStream istream = CreateStream("1.txt");
	printf("%d", GetFromStream(&istream));
	system("pause");
	return 0;
}