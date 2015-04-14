"epoll_create" "libc.so" resolve
: epoll_create ( size -- epfd ) literal 1 libcall ;
1 epoll_create : epfd literal ;
"epoll_ctl" "libc.so" resolve
: epoll_ctl ( epfd op fd event -- err ) literal 4 libcall ;
: EPOLL_CTL_ADD 1 ;
: EPOLL_CTL_DEL 2 ;
: EPOLL_CTL_MOD 3 ;
"malloc" "libc.so" resolve
: malloc ( size -- ptr ) literal 1 libcall ;
12 malloc
: evt literal ;
: epoll_event ( evts ptr fd -- evt ) evt 8 + ! evt 4 + ! evt ! evt ;
: EPOLLIN 1 ;
: EPOLLOUT 4 ;
"open" "libc.so" resolve
: open ( str flags -- fd ) literal 2 libcall ;
: O_RDONLY 0 ;
: watch ( str -- ) >r epfd EPOLL_CTL_ADD r> dup >r O_RDONLY open dup >r EPOLLIN r> r> swap epoll_event epoll_ctl drop ;
: unwatch ( fd -- ) >r epfd EPOLL_CTL_MOD r> 0 0 0 epoll_event epoll_ctl drop ;
"epoll_wait" "libc.so" resolve
: epoll_wait ( epfd events maxevents timeout -- n ) literal 4 libcall ;
"puts" "libc.so" resolve
: puts ( str -- ) literal 1 libcall drop ;
"read" "libc.so" resolve
: read ( fd buf count -- r ) literal 3 libcall ;
"write" "libc.so" resolve
: write ( fd buf count -- w ) literal 3 libcall ;
100 malloc : buf literal ;
: readwrite ( fd -- w ) buf 100 read >r 1 buf r> write ;
: handle ( fd -- fd ) dup readwrite 0 = not zret drop dup unwatch ;
: poll ( -- ) epfd evt 1 -1 epoll_wait zret drop evt 4 + @ puts evt 8 + @ handle recurse ;
