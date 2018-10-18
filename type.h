/*
Now there are macros of types.
Types are:
1. letter_(letter_ | digit)*
2. digit+
3. typenames: char double float int short void
4. operators: () [] -> ! ~ ++ -- + - * & / % + - << >> < <= >= >
			  != == ^ | && || ? : = += -= *= /= &= ^= != <<= >>= 
5. controllers: case break continue const do else for goto if return switch while
6. delimiters: ; {} , \n \r\n
*/
#define VARNAME 0 //variables
#define NUM 1 //numbers
#define TYPENAME 2
#define OPERATOR 3
#define CONTROLLER 4
#define DELIMITER 5
