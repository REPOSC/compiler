#include "type.h"
#include <unordered_map>
#include <string>
#ifndef BASE__32
#define BASE__32

typedef unsigned int SYM_TYPE;
typedef unsigned int TYPE;
typedef unsigned int SYM_NAME;
typedef int INTEGER;
typedef double REAL;
typedef char * VAR_NAME;
typedef void * ADDR;
typedef struct
{
	ADDR address;
	TYPE type;
} VAR_VALUE;
typedef std::unordered_map<VAR_NAME, VAR_VALUE> SYMBOL_TABLE;

typedef union
{
	SYM_NAME sym_name;
	VAR_NAME var_name;
	INTEGER int_value;
	REAL real_value;
} SYM_VALUE;

typedef struct
{
	SYM_TYPE type;
	SYM_VALUE value;
}token;

/* Now there is a token for example. */
token EOF_TOKEN{ EOF_TYPE, 0xff };
token ERR_TOKEN{ ERR_TYPE, 0xff };

#endif