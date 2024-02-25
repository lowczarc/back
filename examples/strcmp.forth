: ddup over over ;
: mdrop 0 do drop loop ;
: sp8b dup 256 / swap 255 and ;

: strcmp
	begin
	ddup
	@ swap @
	sp8b rot sp8b rot swap
	ddup
	= not if - rot rot 2 mdrop leave then
	dup
	0 = if - rot rot 2 mdrop leave then
	over
	0 = if - rot rot 2 mdrop leave then
	2 mdrop
	ddup
	= not if - leave then
	dup
	0 = if - leave then
	over
	0 = if - leave then
	2 mdrop
	1 cells +
	swap
	1 cells +
	again
	rot rot 2 mdrop
;

" abcdef" " tutu" strcmp . cr
" tutu" " abcdef" strcmp . cr
" tatu" " tata" strcmp . cr
" tata" " tatu" strcmp . cr
" tatau" " tata" strcmp . cr
" tata" " tatau" strcmp . cr
" tata" " tata" strcmp . cr
