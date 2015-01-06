%{
#include "token.h"
char *strdup(const char *);
%}
%%
\([^)]*\) { }
\"(\\.|[^\"])*\" { yytext[yyleng - 1] = 0; yylval.string = strdup(yytext + 1); return T_STRING; }
[+-]?(0x[0-9A-Fa-f]+|0[0-7]*|[1-9][0-9]*) { yylval.number = strtol(yytext, NULL, 0); return T_NUMBER; }
[^ \t\n\v]+ { yylval.string = strdup(yytext); return T_WORD; }
\n|. { }
%%
