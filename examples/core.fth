: over ( a b -- a b a ) >r dup r> swap ;
: rot ( a b c -- b c a ) >r swap r> swap ;
: -rot ( a b c -- c a b ) swap >r swap r> ;
