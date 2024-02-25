: ddup over over ;
: mdrop 0 do drop loop ;
: sp8b dup 256 / swap 255 and ;

: strcmp
	begin
	ddup
	@ 255 and swap @ 255 and
	ddup
	-
	dup 0 = not if rot rot drop drop leave then
	rot 0 = if swap drop leave then
	swap 0 = if leave then
	drop
	1 +
	swap
	1 +
	again
	rot rot drop drop
;

42 " abcdef" " tutu" strcmp . ."  " . cr
42 " tutu" " abcdef" strcmp . ."  " . cr
42 " tatu" " tata" strcmp . ."  " . cr
42 " tata" " tatu" strcmp . ."  " . cr
42 " tatau" " tata" strcmp . ."  " . cr
42 " tata" " tatau" strcmp . ."  " . cr
42 " tata" " tata" strcmp . ."  " . cr
