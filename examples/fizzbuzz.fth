"libc.so.6" load "puts" import : puts ( str -- int ) literal 1 libcall ;
: orfizz ( a i -- a|fizz ) :noname "fizz" puts drop ; swap 3 % ?: ;
: orbuzz ( a i -- a|buzz ) :noname "buzz" puts drop ; swap 5 % ?: ;
: orfzbz ( a i -- a|fizzbuzz ) :noname "fizzbuzz" puts drop ; swap 15 % ?: ;
: over ( a b -- a b a ) >r dup r> swap ;
: classify ( i -- i xt ) :noname dup . ; over orfizz over orbuzz over orfzbz ;
: loop ( n i -- n i ) over over < not zret drop classify execute 1 + recurse ;
100 1 loop
