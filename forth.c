#include "forth.h"
#include "lexer.h"
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1024

token stack[STACK_SIZE];

void
push(token t)
{
	stack[++stack[0].number] = t;
}

token
pop(void)
{
	token t = stack[stack[0].number];
	stack[stack[0].number].number = 0;
	stack[0].number--;
	return t;
}

enum state
{
	S_INTERPRET,
	S_COMPILE
};

enum state state;

void *dict;

enum codetype
{
	C_NATIVE,
	C_USER,
	C_TOKEN
};

struct usercode
{
	int size;
	int length;
	struct word **user;
};

void
usercode_append(struct usercode *uc, struct word *w)
{
	uc->length++;
	if(uc->size == 0)
	{
		uc->size = 2;
		uc->user = malloc(uc->size * sizeof(*uc->user));
	}
	else if(uc->length >= uc->size)
	{
		uc->size *= 2;
		uc->user = realloc(uc->user, uc->size * sizeof(*uc->user));
	}
	uc->user[uc->length - 1] = w;
}

struct word
{
	char *name;
	int immediate;
	enum codetype codetype;
	union
	{
		void (*native)(void);
		struct usercode user;
		token token;
	} code;
};

struct word *current_word;

int
cmp(const void *a, const void *b)
{
	return strcmp(((struct word *)a)->name, ((struct word *)b)->name);
}

void
add(void)
{
	token a = pop();
	token b = pop();
	a.number += b.number;
	push(a);
}

void
mul(void)
{
	token a = pop();
	token b = pop();
	a.number *= b.number;
	push(a);
}

void
print(void)
{
	token t = pop();
	printf("%d\n", t.number);
}

void
define(void)
{
	current_word = calloc(1, sizeof(*current_word));
	if(yylex() != T_WORD)
	{
		return;
	}
	current_word->name = yylval.string;
	current_word->codetype = C_USER;
	state = S_COMPILE;
}

void
end(void)
{
	usercode_append(&current_word->code.user, NULL);
	if(current_word->name)
	{
		tsearch(current_word, &dict, cmp);
	}
	else
	{
		token t;
		t.number = current_word;
		push(t);
	}
	state = S_INTERPRET;
	current_word = NULL;
}

struct word primitives[] = 
{
	{"+", 0, C_NATIVE, {add}},
	{"*", 0, C_NATIVE, {mul}},
	{".", 0, C_NATIVE, {print}},
	{":", 1, C_NATIVE, {define}},
	{";", 1, C_NATIVE, {end}},
};

#define arrsz(arr) (sizeof(arr) / sizeof(arr[0]))

void
init(void)
{
	unsigned int i;
	for(i = 0; i < arrsz(primitives); i++)
	{
		tsearch(&primitives[i], &dict, cmp);
	}
}

void
run_word(struct word *w)
{
	struct word **t;
	switch(w->codetype)
	{
	case C_NATIVE:
		w->code.native();
		break;
	case C_USER:
		for(t = w->code.user.user; *t; t++)
		{
			run_word(*t);
		}
		break;
	case C_TOKEN:
		push(w->code.token);
		break;
	}
}

void
interpret_token(enum type type)
{
	struct word **w;
	switch(type)
	{
	case T_STRING:
	case T_NUMBER:
		push(yylval);
		break;
	case T_WORD:
		w = tfind(&yylval, &dict, cmp);
		if(w)
		{
			run_word(*w);
		}
		break;
	}
}

void
compile_token(enum type type)
{
	struct word **w;
	struct word *t;
	switch(type)
	{
	case T_STRING:
	case T_NUMBER:
		t = calloc(1, sizeof(*t));
		t->codetype = C_TOKEN;
		t->code.token = yylval;
		usercode_append(&current_word->code.user, t);
		break;
	case T_WORD:
		w = tfind(&yylval, &dict, cmp);
		if(!w)
		{
			break;
		}
		if((*w)->immediate)
		{
			run_word(*w);
		}
		else
		{
			usercode_append(&current_word->code.user, *w);
		}
		break;
	}
}

void
process_token(enum type type)
{
	switch(state)
	{
	case S_INTERPRET:
		interpret_token(type);
		break;
	case S_COMPILE:
		compile_token(type);
		break;
	}
}
