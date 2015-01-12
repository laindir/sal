#include "forth.h"
#include "lexer.h"
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#define STACK_SIZE 1024

token stack[STACK_SIZE];
token rstack[STACK_SIZE];

void
push(token t)
{
	stack[++stack[0].number] = t;
}

token
top(void)
{
	return stack[stack[0].number];
}

token
pop(void)
{
	token t = stack[stack[0].number];
	stack[stack[0].number].number = 0;
	stack[0].number--;
	return t;
}

void
rpush(token t)
{
	rstack[++rstack[0].number] = t;
}

token
rtop(void)
{
	return rstack[rstack[0].number];
}

token
rpop(void)
{
	token t = rstack[rstack[0].number];
	rstack[rstack[0].number].number = 0;
	rstack[0].number--;
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

struct word *
lookup(const char *s)
{
	struct word **w;
	w = tfind(&s, &dict, cmp);
	if(!w)
	{
		fprintf(stderr, "Word not found: %s\n", yylval.string);
		exit(EXIT_FAILURE);
	}
	return *w;
}

void run_word(struct word *w);

int early_return = 0;

void
zret(void)
{
	token a = pop();
	if(a.number == 0)
	{
		early_return = 1;
	}
	else
	{
		push(a);
	}
}

void
resolve(void)
{
	void *so = dlopen(pop().string, RTLD_LAZY);
	void *f = dlsym(so, pop().string);
	token a;
	a.number = (int)f;
	push(a);
}

void
libcall(void)
{
	int n = pop().number;
	int (*f)(void) = (int (*)(void))pop().number;
	token result;
	int a1;
	int a2;
	int a3;
	int a4;
	int a5;
	int a6;
	int a7;
	switch(n)
	{
	case 7: a7 = pop().number;
	case 6: a6 = pop().number;
	case 5: a5 = pop().number;
	case 4: a4 = pop().number;
	case 3: a3 = pop().number;
	case 2: a2 = pop().number;
	case 1: a1 = pop().number;
	case 0: break;
	}
	switch(n)
	{
	case 0:
		result.number = f();
		break;
	case 1:
		result.number = ((int (*)(int))f)(a1);
		break;
	case 2:
		result.number = ((int (*)(int, int))f)(a1, a2);
		break;
	case 3:
		result.number = ((int (*)(int, int, int))f)(a1, a2, a3);
		break;
	case 4:
		result.number = ((int (*)(int, int, int, int))f)(a1, a2, a3, a4);
		break;
	case 5:
		result.number = ((int (*)(int, int, int, int, int))f)(a1, a2, a3, a4, a5);
		break;
	case 6:
		result.number = ((int (*)(int, int, int, int, int, int))f)(a1, a2, a3, a4, a5, a6);
		break;
	case 7:
		result.number = ((int (*)(int, int, int, int, int, int, int))f)(a1, a2, a3, a4, a5, a6, a7);
		break;
	}
	push(result);
}

void
execute(void)
{
	run_word((struct word *)pop().number);
}

void
tor(void)
{
	token a = pop();
	rpush(a);
}

void
fromr(void)
{
	token a = rpop();
	push(a);
}

void
fetch(void)
{
	token a = pop();
	a.number = *(int *)a.number;
	push(a);
}

void
store(void)
{
	token a = pop();
	token b = pop();
	*(int *)a.number = b.number;
}

void
lt(void)
{
	token a = pop();
	token b = pop();
	b.number = b.number < a.number ? ~0 : 0;
	push(b);
}

void
eq(void)
{
	token a = pop();
	token b = pop();
	b.number = b.number == a.number ? ~0 : 0;
	push(b);
}

void
add(void)
{
	token a = pop();
	token b = pop();
	b.number += a.number;
	push(b);
}

void
sub(void)
{
	token a = pop();
	token b = pop();
	b.number -= a.number;
	push(b);
}

void
mul(void)
{
	token a = pop();
	token b = pop();
	b.number *= a.number;
	push(b);
}

void
divi(void)
{
	token a = pop();
	token b = pop();
	b.number /= a.number;
	push(b);
}

void
mod(void)
{
	token a = pop();
	token b = pop();
	b.number %= a.number;
	push(b);
}

void
and(void)
{
	token a = pop();
	token b = pop();
	b.number &= a.number;
	push(b);
}

void
or(void)
{
	token a = pop();
	token b = pop();
	b.number |= a.number;
	push(b);
}

void
xor(void)
{
	token a = pop();
	token b = pop();
	b.number ^= a.number;
	push(b);
}

void
not(void)
{
	token a = pop();
	a.number = ~a.number;
	push(a);
}

void
drop(void)
{
	(void)pop();
}

void
dupl(void)
{
	token a = pop();
	push(a);
	push(a);
}

void
swap(void)
{
	token a = pop();
	token b = pop();
	push(a);
	push(b);
}

void
print(void)
{
	token t = pop();
	printf("%d\n", t.number);
}

void
immediate(void)
{
	token a = pop();
	struct word *w = calloc(1, sizeof(*w));
	w->codetype = C_TOKEN;
	w->code.token = a;
	usercode_append(&current_word->code.user, w);
}

void
recurse(void)
{
	usercode_append(&current_word->code.user, current_word);
}

char *
read_word(void)
{
	switch(yylex())
	{
	case T_WORD:
		return yylval.string;
	case T_NUMBER:
		fprintf(stderr, "Expected word, got number: %d\n", yylval.number);
		exit(EXIT_FAILURE);
		break;
	case T_STRING:
		fprintf(stderr, "Expected word, got string: \"%s\"\n", yylval.string);
		exit(EXIT_FAILURE);
		break;
	case 0:
		fprintf(stderr, "Unexpected end of file\n");
		exit(EXIT_FAILURE);
		break;
	}
	return NULL;
}

void
define(void)
{
	if(state == S_COMPILE)
	{
		fprintf(stderr, "Nested definitions are not allowed\n");
		exit(EXIT_FAILURE);
	}
	current_word = calloc(1, sizeof(*current_word));
	current_word->name = read_word();
	current_word->codetype = C_USER;
	state = S_COMPILE;
}

void
noname(void)
{
	current_word = calloc(1, sizeof(*current_word));
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
		t.number = (int)current_word;
		push(t);
	}
	state = S_INTERPRET;
	current_word = NULL;
}

void
tick(void)
{
	token a;
	a.number = (int)lookup(read_word());
	push(a);
}

void
display_nodename(const void *nodep, const VISIT which, const int depth)
{
	struct word *const *w = nodep;
	(void)depth;
	switch(which)
	{
	case postorder:
	case leaf:
		printf("%s ", (*w)->name);
		break;
	case preorder:
	case endorder:
		break;
	}
}

void
words(void)
{
	twalk(dict, display_nodename);
	printf("\n");
}

void
see(void)
{
	struct word *w;
	struct word **ip;
	w = lookup(read_word());
	switch(w->codetype)
	{
	case C_NATIVE:
		printf("(native)\n");
		break;
	case C_USER:
		printf(": %s ", w->name);
		for(ip = w->code.user.user; *ip; ip++)
		{
			if(*ip == w)
			{
				printf("recurse ");
			}
			else if((*ip)->codetype == C_TOKEN)
			{
				printf("%d ", (*ip)->code.token.number);
			}
			else
			{
				printf("%s ", (*ip)->name);
			}
		}
		printf(";\n");
		break;
	case C_TOKEN:
		break;
	}
}

struct word primitives[] = 
{
	{"zret", 0, C_NATIVE, {zret}},
	{">r", 0, C_NATIVE, {tor}},
	{"r>", 0, C_NATIVE, {fromr}},
	{"<", 0, C_NATIVE, {lt}},
	{"=", 0, C_NATIVE, {eq}},
	{"+", 0, C_NATIVE, {add}},
	{"-", 0, C_NATIVE, {sub}},
	{"*", 0, C_NATIVE, {mul}},
	{"/", 0, C_NATIVE, {divi}},
	{"%", 0, C_NATIVE, {mod}},
	{".", 0, C_NATIVE, {print}},
	{":", 1, C_NATIVE, {define}},
	{";", 1, C_NATIVE, {end}},
	{"imm", 1, C_NATIVE, {immediate}},
	{"recurse", 1, C_NATIVE, {recurse}},
	{"drop", 0, C_NATIVE, {drop}},
	{"dup", 0, C_NATIVE, {dupl}},
	{"swap", 0, C_NATIVE, {swap}},
	{"resolve", 0, C_NATIVE, {resolve}},
	{"libcall", 0, C_NATIVE, {libcall}},
	{"execute", 0, C_NATIVE, {execute}},
	{":noname", 1, C_NATIVE, {noname}},
	{"'", 1, C_NATIVE, {tick}},
	{"and", 0, C_NATIVE, {and}},
	{"or", 0, C_NATIVE, {or}},
	{"xor", 0, C_NATIVE, {xor}},
	{"not", 0, C_NATIVE, {not}},
	{"@", 0, C_NATIVE, {fetch}},
	{"!", 0, C_NATIVE, {store}},
	{"words", 0, C_NATIVE, {words}},
	{"see", 1, C_NATIVE, {see}},
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
	struct word **ip;
	switch(w->codetype)
	{
	case C_NATIVE:
		w->code.native();
		break;
	case C_USER:
		for(ip = w->code.user.user; *ip && !early_return; ip++)
		{
			if(w == *ip && *(ip + 1) == 0)
			{
				ip = w->code.user.user;
			}
			run_word(*ip);
		}
		early_return = 0;
		break;
	case C_TOKEN:
		push(w->code.token);
		break;
	}
}

void
interpret_token(enum type type)
{
	struct word *w;
	switch(type)
	{
	case T_STRING:
	case T_NUMBER:
		push(yylval);
		break;
	case T_WORD:
		w = lookup(yylval.string);
		run_word(w);
		break;
	}
}

void
compile_token(enum type type)
{
	struct word *w;
	switch(type)
	{
	case T_STRING:
	case T_NUMBER:
		w = calloc(1, sizeof(*w));
		w->codetype = C_TOKEN;
		w->code.token = yylval;
		usercode_append(&current_word->code.user, w);
		break;
	case T_WORD:
		w = lookup(yylval.string);
		if(w->immediate)
		{
			run_word(w);
		}
		else
		{
			usercode_append(&current_word->code.user, w);
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
