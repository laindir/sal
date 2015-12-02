#include "forth.h"
#include "lexer.h"
char *strdup(const char *);
int getopt(int argc, char * const argv[], const char *optstring);
extern char *optarg;
extern int optind;

int
yywrap(void)
{
	if(rtop().number)
	{
		yyin = (FILE *)rpop().number;
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
	while((t = getopt(argc, argv, "f:a")) != -1)
	{
		switch(t)
		{
		case 'a':
			alt = 1;
			break;
		case 'f':
			yyin = fopen(optarg, "r");
			break;
		}
	}
	init();
	for(t = optind; t < argc; t++)
	{
		errno = 0;
		yylval.number = strtol(argv[t], NULL, 0);
		if(errno == 0 && strpbrk(argv[t], "0123456789abcdefABCDEF"))
		{
			process_token(T_NUMBER);
		}
		else if(argv[t][0] == '"')
		{
			int l = strlen(argv[t]);
			yylval.string = strdup(argv[t] + 1);
			yylval.string[l - 2] = 0;
			process_token(T_STRING);
		}
		else
		{
			yylval.string = argv[t];
			process_token(T_WORD);
		}
	}
	while((t = yylex()))
	{
		process_token(t);
	}
	return top().number;
}
