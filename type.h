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

/* Here are Root Types */
#define TERMINATOR 0
#define VARNAME 1 /* variables */
#define INT_NUM 2 /* integer numbers */
#define REAL_NUM 3 /* float numbers */
#define STRING 4 /* string */
#define TYPENAME 5
#define OPERATOR 6
#define CONTROLLER 7
#define DELIMITER 8
#define MAIN 9
#define UNTERMINATOR 10 /* unterminators */
#define ABSTRACT_NUM 11
#define ABSTRACT_VAR 12
#define STRANGE_TOKEN 13
#define NULL_TOKEN 14

#define EOF_TYPE 0xff
#define ERR_TYPE 0xfe


/* Here are Typenames */
#define Type_CHAR 0
#define Type_DOUBLE 1
#define Type_FLOAT 2
#define Type_INT 3
#define Type_SHORT 4
#define Type_VOID 5
#define Type_Pointer 6


/* Here are Operators */
#define Operator_LEFT_BRACKET 0
#define Operator_RIGHT_BRACKET 1
#define Operator_LEFT_SQUARE_BRACKET 2
#define Operator_RIGHT_SQUARE_BRACKET 3
#define Operator_ARROW 4
#define Operator_EXCLAMATION_MARK 5
#define Operator_WAVY_LINE 6
#define Operator_PLUS_PLUS 7
#define Operator_MINUS_MINUS 8
#define Operator_PLUS 9
#define Operator_MINUS 10
#define Operator_MULTIPLE 11
#define Operator_AND 12
#define Operator_DIVISION 13
#define Operator_REMAINDER 14
#define Operator_POSITIVE 15
#define Operator_NEGATIVE 16
#define Operator_LEFT_SHIFT 17
#define Operator_RIGHT_SHIFT 18
#define Operator_LESS 19
#define Operator_LESS_OR_EQUAL 20
#define Operator_GREATER_OR_EQUAL 21
#define Operator_GREATER 22
#define Operator_NOT_EQUAL 23
#define Operator_EQUAL 24
#define Operator_POWER 25
#define Operator_OR 26
#define Operator_AND_AND 27
#define Operator_OR_OR 28
#define Operator_QUESTION_MARK 29
#define Operator_COLON 30
#define Operator_ASSIGNMENT 31
#define Operator_PLUS_ASSIGNMENT 32
#define Operator_MINUS_ASSIGNMENT 33
#define Operator_MULTIPLE_ASSIGNMENT 34
#define Operator_DIVISION_ASSIGNMENT 35
#define Operator_AND_ASSIGNMENT 36
#define Operator_OR_ASSIGNMENT 37
#define Operator_POWER_ASSIGNMENT 38
#define Operator_REMAINDER_ASSIGNMENT 39
#define Operator_LEFT_SHIFT_ASSIGNMENT 40
#define Operator_RIGHT_SHIFT_ASSIGNMENT 41
#define Operator_DOT 42


/* Here are Controllers */
#define Controller_CASE 0
#define Controller_BREAK 1
#define Controller_CONTINUE 2
#define Controller_CONST 3
#define Controller_DO 4
#define Controller_ELSE 5
#define Controller_FOR 6
#define Controller_GOTO 7
#define Controller_IF 8
#define Controller_RETURN 9
#define Controller_SWITCH 10
#define Controller_WHILE 11
#define Controller_PRINT 12
#define Controller_INPUT 13
#define Controller_MAIN 14


/* Here are Delimiters */
#define Delimiter_SEMICOLON 0
#define Delimiter_LEFT_BRACE 1
#define Delimiter_RIGHT_BRACE 2
#define Delimiter_COMMA 3
#define Delimiter_LINE_BREAK 4
#define Delimiter_CARRIAGE_RETURN 5

#endif
