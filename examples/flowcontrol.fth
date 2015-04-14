: else ( a bool -- a|0 ) zret drop drop 0 ;
: lognot ( bool -- !bool ) 0 not swap zret drop not ;
: ?: ( a b bool -- a|b ) dup >r lognot swap r> else swap >r swap r> else or ;
