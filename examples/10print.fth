"libc.so.6" load : libc literal ;
libc "rand" import : rand literal 0 libcall ;
libc "putchar" import : putchar literal 1 libcall ;
: 10print "\/" rand 1 and + @ 255 and putchar drop 1 - zret recurse ;
800 10print
