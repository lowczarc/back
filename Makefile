CC=gcc
CC_FLAGS= -Wall -Wextra -Werror -I includes

all: forthc

OBJS=main.o ast/lexer.o ast/dbg.o ast/tree.o ast/ast.o
HEADERS=includes/ast.h

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CC_FLAGS)

forthc: $(OBJS) $(HEADERS)
	$(CC) $(OBJS) -o $@ $(CC_FLAGS)

%: %.forth
	(./forthc $< > $@.s) && nasm -f elf64 $@.s && gcc $@.o -static -o $@

clean:
	rm -f forthc
	rm -f *.o
	rm -f *.s

.PHONY: clean
