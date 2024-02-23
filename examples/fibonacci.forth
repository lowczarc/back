: fibonacci
	dup 1 > if
		dup
		2 - fibonacci
		swap
		1 - fibonacci
		+
	then
;

25 0 do i fibonacci . cr loop
