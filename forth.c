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
	if(stack[0].number + 1 == STACK_SIZE)
	{
		fprintf(stderr, "Stack overflow\n");
		return;
	}
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
	if(stack[0].number == 0)
	{
		fprintf(stderr, "Stack underflow\n");
		return t;
	}
	stack[stack[0].number].number = 0;
	stack[0].number--;
	return t;
}

void
rpush(token t)
{
	if(rstack[0].number + 1 == STACK_SIZE)
	{
		fprintf(stderr, "Return stack overflow\n");
		return;
	}
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
	if(rstack[0].number == 0)
	{
		fprintf(stderr, "Return stack underflow\n");
		return t;
	}
	rstack[rstack[0].number].number = 0;
	rstack[0].number--;
	return t;
}

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
	if(!s)
	{
		fprintf(stderr, "Word not found: %s\n", yylval.string);
		return NULL;
	}
	w = tfind(&s, &dict, cmp);
	if(!w)
	{
		fprintf(stderr, "Word not found: %s\n", yylval.string);
		return NULL;
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
load(void)
{
	void *so;
	token a;
	char *msg;
	dlerror();
	so = dlopen(pop().string, RTLD_LAZY);
	msg = dlerror();
	if(msg)
	{
		fprintf(stderr, "%s\n", msg);
	}
	a.number = (int)so;
	push(a);
}

void
import(void)
{
	token a = pop();
	token b = pop();
	void *f;
	char *msg;
	dlerror();
	f = dlsym(b.string, a.string);
	msg = dlerror();
	if(msg)
	{
		fprintf(stderr, "%s\n", msg);
	}
	a.number = (int)f;
	push(a);
}

union nary
{
	int n;
	int (*f0)(void);
	int (*f1)(int);
	int (*f2)(int,int);
	int (*f3)(int,int,int);
	int (*f4)(int,int,int,int);
	int (*f5)(int,int,int,int,int);
	int (*f6)(int,int,int,int,int,int);
	int (*f7)(int,int,int,int,int,int,int);
};

void
libcall(void)
{
	int n = pop().number;
	union nary f;
	token result;
	int a[7];
	int i;
	f.n = pop().number;
	for(i = n; i--;)
	{
		a[i] = pop().number;
	}
	switch(n)
	{
	case 0:
		result.number = f.f0();
		break;
	case 1:
		result.number = f.f1(a[0]);
		break;
	case 2:
		result.number = f.f2(a[0],a[1]);
		break;
	case 3:
		result.number = f.f3(a[0],a[1],a[2]);
		break;
	case 4:
		result.number = f.f4(a[0],a[1],a[2],a[3]);
		break;
	case 5:
		result.number = f.f5(a[0],a[1],a[2],a[3],a[4]);
		break;
	case 6:
		result.number = f.f6(a[0],a[1],a[2],a[3],a[4],a[5]);
		break;
	case 7:
		result.number = f.f7(a[0],a[1],a[2],a[3],a[4],a[5],a[6]);
		break;
	default:
		fprintf(stderr, "libcall currently limited to 7 arguments\n");
		result.number = 0;
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
literal(void)
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

void
include(void)
{
	token a = pop();
	token b;
	b.number = (int)yyin;
	rpush(b);
	yyin = fopen(a.string, "r");
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
		break;
	case T_STRING:
		fprintf(stderr, "Expected word, got string: \"%s\"\n", yylval.string);
		break;
	case 0:
		fprintf(stderr, "Unexpected end of file\n");
		break;
	}
	return NULL;
}

int compilation_depth;

struct word *noname_word;

void
define(void)
{
	if(compilation_depth == 0)
	{
		current_word = calloc(1, sizeof(*current_word));
		current_word->name = read_word();
		current_word->codetype = C_USER;
	}
	else
	{
		fprintf(stderr, "Nested definitions are not allowed, treating as :noname\n");
		usercode_append(&current_word->code.user, noname_word);
	}
	compilation_depth++;
}

void
noname(void)
{
	if(compilation_depth == 0)
	{
		current_word = calloc(1, sizeof(*current_word));
		current_word->codetype = C_USER;
	}
	else
	{
		usercode_append(&current_word->code.user, noname_word);
	}
	compilation_depth++;
}

struct word *end_word;

void
end(void)
{
	compilation_depth--;
	if(compilation_depth == 0)
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
		current_word = NULL;
	}
	else
	{
		usercode_append(&current_word->code.user, end_word);
	}
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
	if(!w)
	{
		return;
	}
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
	{";", 1024, C_NATIVE, {end}},
	{"literal", 1, C_NATIVE, {literal}},
	{"recurse", 1, C_NATIVE, {recurse}},
	{"drop", 0, C_NATIVE, {drop}},
	{"dup", 0, C_NATIVE, {dupl}},
	{"swap", 0, C_NATIVE, {swap}},
	{"resolve", 0, C_NATIVE, {resolve}},
	{"load", 0, C_NATIVE, {load}},
	{"import", 0, C_NATIVE, {import}},
	{"libcall", 0, C_NATIVE, {libcall}},
	{"execute", 0, C_NATIVE, {execute}},
	{":noname", 1024, C_NATIVE, {noname}},
	{"'", 1, C_NATIVE, {tick}},
	{"and", 0, C_NATIVE, {and}},
	{"or", 0, C_NATIVE, {or}},
	{"xor", 0, C_NATIVE, {xor}},
	{"not", 0, C_NATIVE, {not}},
	{"@", 0, C_NATIVE, {fetch}},
	{"!", 0, C_NATIVE, {store}},
	{"words", 0, C_NATIVE, {words}},
	{"see", 1, C_NATIVE, {see}},
	{"include", 0, C_NATIVE, {include}},
};

struct word alt_primitives[] = 
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
	{"{:", 1, C_NATIVE, {define}},
	{"}", 1024, C_NATIVE, {end}},
	{"?", 1, C_NATIVE, {literal}},
	{"recurse", 1, C_NATIVE, {recurse}},
	{"drop", 0, C_NATIVE, {drop}},
	{"dup", 0, C_NATIVE, {dupl}},
	{"swap", 0, C_NATIVE, {swap}},
	{"load", 0, C_NATIVE, {load}},
	{"import", 0, C_NATIVE, {import}},
	{"libcall", 0, C_NATIVE, {libcall}},
	{"->", 0, C_NATIVE, {execute}},
	{"{", 1024, C_NATIVE, {noname}},
	{"'", 1, C_NATIVE, {tick}},
	{"&", 0, C_NATIVE, {and}},
	{"|", 0, C_NATIVE, {or}},
	{"^", 0, C_NATIVE, {xor}},
	{"~", 0, C_NATIVE, {not}},
	{"@", 0, C_NATIVE, {fetch}},
	{"!", 0, C_NATIVE, {store}},
	{"words", 0, C_NATIVE, {words}},
	{"see", 1, C_NATIVE, {see}},
	{"include", 0, C_NATIVE, {include}},
};

int alt;

#define arrsz(arr) (sizeof(arr) / sizeof(arr[0]))

void
init(void)
{
	unsigned int i;
	unsigned int n = alt ? arrsz(alt_primitives) : arrsz(primitives);
	struct word *vocab = alt ? alt_primitives : primitives;
	for(i = 0; i < n; i++)
	{
		tsearch(&vocab[i], &dict, cmp);
	}
	if(alt)
	{
		noname_word = lookup("{");
		end_word = lookup("}");
	}
	else
	{
		noname_word = lookup(":noname");
		end_word = lookup(";");
	}
}

void
do_value(void)
{
	if(compilation_depth == 0)
	{
		push(yylval);
	}
	else
	{
		struct word *w;
		w = calloc(1, sizeof(*w));
		w->codetype = C_TOKEN;
		w->code.token = yylval;
		usercode_append(&current_word->code.user, w);
	}
}

void
do_word(struct word *w)
{
	if(compilation_depth <= w->immediate)
	{
		run_word(w);
	}
	else
	{
		usercode_append(&current_word->code.user, w);
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
			do_word(*ip);
		}
		early_return = 0;
		break;
	case C_TOKEN:
		push(w->code.token);
		break;
	}
}

void
process_token(enum type type)
{
	struct word *w;
	switch(type)
	{
	case T_NUMBER:
	case T_STRING:
		do_value();
		break;
	case T_WORD:
		w = lookup(yylval.string);
		if(!w)
		{
			return;
		}
		do_word(w);
		break;
	}
}
