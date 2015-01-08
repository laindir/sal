#include "forth.h"
#include "lexer.h"

int fargc;
char **fargv;

int
yywrap(void)
{
	static int i;
	if(++i < fargc)
	{
		yyin = strcmp(fargv[i], "-") == 0 ? stdin : fopen(fargv[i], "r");
		return 0;
	}
	else
	{
		return 1;
	}
}

int
main(int argc, char *argv[])
{
	int t;
	init();
	if(argc > 1)
	{
		fargc = argc;
		fargv = argv;
		yywrap();
	}
	while((t = yylex()))
	{
		process_token(t);
	}
	return top().number;
}
