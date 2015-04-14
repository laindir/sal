: true ( a b -- a ) drop ;
: false ( a b -- b ) swap drop ;
: ifthenelse ( else then condition -- ) execute execute ;
