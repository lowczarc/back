all: forthc

%.o: %.c
	gcc -c -o $@ $<

forthc: main.o
	gcc $< -o $@

%: %.forth
	(./forthc $< > $@.s) && nasm -f elf64 $@.s && gcc $@.o -static -o $@

clean:
	rm -f forthc
	rm -f *.o
	rm -f *.s

.PHONY: clean
