: Y
	:noname dup
		:noname literal literal execute execute ;
	literal execute ;
dup execute ;
:noname
	:noname 1 swap zret swap drop dup 1 - literal execute * ;
;
Y : fact ( n -- n! ) literal execute ;
