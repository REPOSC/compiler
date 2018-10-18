#include <unordered_map>
#include <string>

typedef unsigned int SYM_TYPE;
typedef unsigned int TYPE;
typedef unsigned int SYM_NAME;
typedef std::string VAR_NAME;
typedef void * ADDR;
typedef struct
{
	ADDR address;
	TYPE type;
};
typedef std::unordered_map<VAR_NAME, ADDR> SYMBOL_TABLE;

typedef union
{
	SYM_NAME name;
	VAR_NAME name;
} SYM_VALUE;

typedef struct
{
	SYM_TYPE type;
	SYM_VALUE value;
}token;