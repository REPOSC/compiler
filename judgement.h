#ifndef JUDGEMENT__32
#define JUDGEMENT__32

bool check_if_leu(char x)
{
	return x >= 'a' && x <= 'z' || x >= 'A' && x <= 'Z' || x == '_';
}

bool check_if_number(char x)
{
	return x >= '0' && x <= '9';
}

bool check_if_led(char x)
{
	return check_if_leu(x) || check_if_number(x);
}

bool check_if_blank(char x)
{
	return x == ' ' || x == '\t' || x == '\n' || x == '\r';
}

bool is_sep(char x) { return x == ' ' || x == '\t' || x == '\f'; };
bool is_end(char x) { return x == '\n' || x == '\0'; }
bool is_alpha(char x) { return isalpha(x); }

#endif
