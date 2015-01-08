: fib ( n -- fn ) 1 swap zret -1 + zret swap drop dup -1 + recurse swap recurse + ;
: prange ( i n -- i n ) over over < zret drop over . swap 1 + swap recurse ;
: fact ( n -- n! ) 1 swap zret swap drop dup -1 + recurse * ;
