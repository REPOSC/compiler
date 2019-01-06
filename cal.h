#include <string>

template <class T>
inline T cal(T a, T b, std::string op){
	switch (op[0]){
	case '+': return a + b;
	case '*': return a * b;
	case '-': return a - b;
	case '/': return a / b;
	case '>': 
		switch (op[1]){
		case '=': return a >= b;
		default: return a > b;
		}
	case '<':
		switch (op[1]){
		case '=': return a <= b;
		default: return a < b;
		}
	case '=':
		return a == b;
	case '!':
		return a != b;
	case '|':
		return a || b;
	case '&':
		return a && b;
	}
}