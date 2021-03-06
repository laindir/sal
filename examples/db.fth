"malloc" "libc.so" resolve
: malloc ( size -- ptr ) literal 1 libcall ;

"db_create" "libdb.so" resolve
: db_create ( &db 0 0 -- err ) literal 3 libcall ;

"strlen" "libc.so" resolve
: strlen ( str -- len ) literal 1 libcall ;
"memset" "libc.so" resolve
: memset ( s c n -- s ) literal 3 libcall ;

"puts" "libc.so" resolve
: puts ( str -- ) literal 1 libcall drop ;

: dbt ( -- ptr ) 28 malloc 0 28 memset ;

4 malloc
: &db literal ;
&db 0 0 db_create drop
&db @ : db literal ;

db 532 + @
: __db_open ( db 0 path 0 type flags 0 -- err ) literal 7 libcall ;
: db_open ( db path type flags -- err ) >r >r >r 0 r> 0 r> r> 0 __db_open ;
: DB_HASH 2 ;
: DB_CREATE 1 ;
db 356 + @
: __db_get ( db 0 &key &val 0 -- err ) literal 5 libcall ;
: db_get ( db &key &val -- err ) >r >r 0 r> r> 0 __db_get ;

db ".db" DB_HASH DB_CREATE db_open drop
dbt : key ( str -- &key ) dup >r strlen 1 + r> literal dup >r ! r> dup >r 4 + ! r> ;
dbt : val literal ;
: get ( keystr -- valstr ) db swap key val db_get drop val @ ;
