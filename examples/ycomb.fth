: Y
	:noname dup
		:noname imm imm execute execute ;
	imm execute ;
dup execute ;
:noname
	:noname 1 swap zret swap drop dup 1 - imm execute * ;
;
Y : fact ( n -- n! ) imm execute ;
