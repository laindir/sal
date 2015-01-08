CFLAGS=-Wall -Wextra -pedantic -ansi -g

main: LDLIBS=-ldl

main: lexer.o forth.o

main.o: forth.h lexer.h

forth.o: forth.h token.h

lexer.o: token.h

lexer: LDLIBS=-ll

%.c %.h: %.l
	lex --outfile=$*.c --header-file=$*.h $<

clean:
	rm -f *.o lexer.h lexer.c

.PHONY: clean
