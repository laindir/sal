"dlopen" "libdl.so" resolve
: load ( filename -- handle ) 1 ( RTLD_LAZY ) literal 2 libcall ;
"dlsym" "libdl.so" resolve
: import ( handle symbol -- cfunction ) literal 2 libcall ;
"libc.so" load : libc literal ;
libc "puts" import : puts literal 1 libcall drop ;
"Hello, World!" puts
"libdl.so" load : libdl literal ;
libdl "dlclose" import : dlclose literal 1 libcall ;
