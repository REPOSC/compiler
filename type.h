/*
Now there are macros of types.
Types are:
1. letter_(letter_ | digit)*
2. digit+
3. typenames: char double float int short void
4. operators: () [] ~ ? : .
			  -> ++ -- >> << && ||
			  +  -  *  /  %  <  >  !  =  &  |  ^  <<  >>
			  += -= *= /= %= <= >= != == &= |= ^= <<= >>=
5. controllers: case break continue const do else for goto if return switch while
6. delimiters: ; {} , \n \r\n
*/

#ifndef TYPE__32
#define TYPE__32

#define VARNAME 0 //variables
#define INT_NUM 1 //integer numbers
#define REAL_NUM 2 //float numbers
#define TYPENAME 3
#define OPERATOR 4
#define CONTROLLER 5
#define DELIMITER 6

#define EOF_TYPE 0xff
#define ERR_TYPE 0xfe

#endif