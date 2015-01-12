"id3_file_open" "libid3tag.so" resolve
: id3_file_open ( path how -- file ) literal 2 libcall ;
: ID3_FILE_MODE_READONLY 0 ;
"id3_file_tag" "libid3tag.so" resolve
: id3_file_tag ( file -- tag ) literal 1 libcall ;
"id3_tag_findframe" "libid3tag.so" resolve
: id3_tag_findframe ( tag whichframe 0 -- frame ) literal 3 libcall ;
: ID3_FRAME_TITLE "TIT2" ;
: ID3_FRAME_ARTIST "TPE1" ;
"id3_frame_field" "libid3tag.so" resolve
: id3_frame_field ( frame 1 -- field ) literal 2 libcall ;
"id3_field_getstrings" "libid3tag.so" resolve
: id3_field_getstrings ( field 0 -- ucs4 ) literal 2 libcall ;
"id3_ucs4_utf8duplicate" "libid3tag.so" resolve
: id3_ucs4_utf8duplicate ( ucs4 -- utf8 ) literal 1 libcall ;
"printf" "libc.so" resolve
: prints ( str -- ) "%s" swap literal 2 libcall drop ;

: gettag ( path -- tag ) ID3_FILE_MODE_READONLY id3_file_open id3_file_tag ;
: getfield ( frame -- field ) 1 id3_frame_field ;
: getstring ( field -- string ) 0 id3_field_getstrings id3_ucs4_utf8duplicate ;
: getartistframe ( tag -- artistframe ) ID3_FRAME_ARTIST 0 id3_tag_findframe ;
: gettitleframe ( tag -- titleframe ) ID3_FRAME_TITLE 0 id3_tag_findframe ;
: getartist ( tag -- artist ) getartistframe getfield getstring ;
: gettitle ( tag -- title ) gettitleframe getfield getstring ;

: dofile ( path -- ) gettag dup getartist prints " - " prints gettitle prints "
" prints ;
