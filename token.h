enum type
{
	T_NUMBER = 257,
	T_STRING = 258,
	T_WORD = 259
};

typedef union
{
	int number;
	char *string;
} token;

token yylval;
