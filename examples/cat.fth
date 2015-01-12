"malloc" "libc.so" resolve : malloc literal 1 libcall ;
"read" "libc.so" resolve : read literal 3 libcall ;
"write" "libc.so" resolve : write literal 3 libcall ;
: bufsz 100 ;
bufsz malloc
: buffer literal ;
: doread ( -- r ) 0 buffer bufsz read ;
: dowrite ( r -- w ) >r 1 buffer r> write ;
: loop doread dowrite zret drop recurse ;
loop
