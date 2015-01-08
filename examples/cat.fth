"malloc" "libc.so" resolve : malloc imm 1 libcall ;
"read" "libc.so" resolve : read imm 3 libcall ;
"write" "libc.so" resolve : write imm 3 libcall ;
: bufsz 100 ;
bufsz malloc
: buffer imm ;
: doread ( -- r ) 0 buffer bufsz read ;
: dowrite ( r -- w ) >r 1 buffer r> write ;
: loop doread dowrite zret drop recurse ;
loop
