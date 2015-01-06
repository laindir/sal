#include "forth.h"
#include "lexer.h"

int
yywrap(void)
{
	return 1;
}

int
main(void)
{
	int t;
	init();
	while((t = yylex()))
	{
		process_token(t);
	}
	return 0;
}
