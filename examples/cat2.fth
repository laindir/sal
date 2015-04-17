: times ( xt n -- ) :noname zret 1 - >r dup >r execute r> r> recurse ; execute drop ;
: repeat ( xt -- ) :noname dup >r execute r> swap zret drop recurse ; execute drop ;
: over ( a b -- a b a ) >r dup r> swap ;
: 2dup ( a b -- a b a b ) over over ;
: rot ( a b c -- b c a ) >r swap r> swap ;
: -rot ( a b c -- c a b ) swap >r swap r> ;
: 3dup ( a b c -- a b c a b c ) dup >r -rot dup >r -rot dup >r -rot r> r> r> ;

"libc.so.6" load : libc literal ;
libc "open" import
	: open ( file mode -- fd ) literal 2 libcall ;
libc "close" import
	: close ( fd -- err ) literal 1 libcall ;
libc "read" import
	: read ( fd buf count -- r ) literal 3 libcall ;
libc "write" import
	: write ( fd buf count -- w ) literal 3 libcall ;
libc "malloc" import
	: malloc ( size -- ptr ) literal 1 libcall ;
libc "free" import
	: free ( ptr -- ) literal 1 libcall drop ;

: cat ( file -- )
	0 open
	100 malloc
	2dup 100
	:noname ( fd buf count -- fd buf count r )
		3dup read
		>r over 1 swap r>
		write
	; repeat
	drop free close
;
