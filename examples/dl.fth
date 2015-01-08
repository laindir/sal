"dlopen" "libdl.so" resolve
: dlopen ( filename flag -- handle ) imm 2 libcall ;
: RTLD_LAZY 0x00001 ;
: from ( filename -- handle ) RTLD_LAZY dlopen ;

"dlsym" "libdl.so" resolve
: dlsym ( handle symbol -- ptr ) imm 2 libcall ;
: import dlsym ;

"libc.so" from
: libc imm ;

libc "exit" import : exit ( code -- ) imm 1 libcall ;

2 exit
