#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct keyword {
	char *word;
	char *compiled;
};

#define count(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))

struct keyword KEYWORDS[] = {
	{ "+", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tadd ax, bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "-", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tsub ax, bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "*", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tmul ax, bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "/", "\txor rdx, rdx\n\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tdiv bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "mod", "\txor rdx, rdx\n\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tdiv bx\n\tmov [r8], dx\n\tadd r8, 16" },
	{ "/mod", "\txor rdx, rdx\n\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tdiv bx\n\t\tmov [r8], dx\n\tadd r8, 16\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "and", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tand ax, bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "or", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\tor ax, bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "or", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\txor ax, bx\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "not", "\tsub r8, 16\n\tmov ax, [r8]\n\txor cx, cx\n\txor ax, 0\n\tsetz cl\n\tmov [r8], cx\n\tadd r8, 16" },
	{ "=", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\txor cx, cx\n\txor ax, bx\n\tsetz cl\n\tmov [r8], cx\n\tadd r8, 16" },
	{ ">", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\txor cx, cx\n\tcmp ax, bx\n\tsetg cl\n\tmov [r8], cx\n\tadd r8, 16" },
	{ "<", "\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov ax, [r8]\n\txor cx, cx\n\tcmp bx, ax\n\tsetg cl\n\tmov [r8], cx\n\tadd r8, 16" },
	{ ".", "\tsub r8, 16\n\txor rsi, rsi\n\tmov si, [r8]\n\txor rax, rax\n\tlea rdi, [rel dot_str]\n\tpush r10\n\tpush r9\n\tpush r8\n\tcall printf wrt ..plt\n\tpop r8\n\tpop r9\n\tpop r10" },
	{ "emit", "\tsub r8, 16\n\txor rsi, rsi\n\tmov si, [r8]\n\txor rax, rax\n\tlea rdi, [rel emit_str]\n\tpush r10\n\tpush r9\n\tpush r8\n\tcall printf wrt ..plt\n\tpop r8\n\tpop r9\n\tpop r10" },
	{ "cr", "\txor rax, rax\n\tlea rdi, [rel cr_str]\n\tpush r10\n\tpush r9\n\tpush r8\n\tcall printf wrt ..plt\n\tpop r8\n\tpop r9\n\tpop r10" },
	{ "swap", "\tsub r8, 16\n\tmov ax, [r8]\n\tsub r8, 16\n\tmov bx, [r8]\n\tmov [r8], ax\n\tadd r8, 16\n\tmov [r8], bx\n\tadd r8, 16" },
	{ "drop", "\tsub r8, 16\n\tmov ax, [r8]" },
	{ "dup", "\tsub r8, 16\n\tmov ax, [r8]\n\tmov [r8], ax\n\tadd r8, 16\n\tmov [r8], ax\n\tadd r8, 16" },
	{ "over", "\tsub r8, 16\n\tmov ax, [r8]\n\tsub r8, 16\n\tmov bx, [r8]\n\tmov [r8], bx\n\tadd r8, 16\n\tmov [r8], ax\n\tadd r8, 16\n\tmov [r8], bx\n\tadd r8, 16" },
	{ "rot", "\tsub r8, 16\n\tmov ax, [r8]\n\tsub r8, 16\n\tmov bx, [r8]\n\tsub r8, 16\n\tmov cx, [r8]\n\tmov [r8], bx\n\tadd r8, 16\n\tmov [r8], ax\n\tadd r8, 16\n\tmov [r8], cx\n\tadd r8, 16" },
	{ "i", "\t mov [r8], r9w\n\tadd r8, 16" },
};

char *get_next_word(FILE *fd) {
	static char buffer[256] = {0}; // The words have a maximum size of 255 characters
	buffer[255] = 0;

	int i = 0;
	char c = 0;
	while (!feof(fd)) {
		c = fgetc(fd);
		if (feof(fd) || c == 0 || c == '\n' || c == ' ' || c == '\r' || c == '\t') {
			if (i == 0) {
				continue;
			}
			break;
		}
		if (i == 255) {
			fprintf(stderr, "Words should never be > 255 in length.");
			_exit(-1);
		}

		buffer[i] = c;

		i++;
	}

	if (i == 0) {
		return NULL;
	}

	buffer[i] = 0;

	return buffer;
}

typedef struct string_list {
	char *str;
	struct string_list *next;
} stringlist_t;

stringlist_t *STRINGS = NULL;
int STRINGS_LEN = 0;

void get_string(FILE *fd) {
	static char buffer[1024] = {0}; // The strings have a maximum size of 1023 characters
	buffer[1023] = 0;

	int i = 0;
	char c = 0;
	while (!feof(fd)) {
		c = fgetc(fd);
		if (feof(fd) || c == 0) {
			fprintf(stderr, "Unexpected EOF inside of string");
			_exit(-1);
		}
		if (c == '"') {
			break;
		}


		if (i == 1023) {
			fprintf(stderr, "Strings should never be > 1023 in length.");
			_exit(-1);
		}
		buffer[i] = c;

		i++;
	}

	buffer[i] = 0;

	size_t len = strnlen(buffer, 1024);
	char *str = malloc(len);

	stringlist_t *new_string = malloc(sizeof(stringlist_t));
	new_string->str = strncpy(str, buffer, len);
	new_string->next = STRINGS;
	STRINGS = new_string;
	STRINGS_LEN++;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: forthc [source.forth]\n\t");
		return -1;
	}

	FILE *src_fd = fopen(argv[1], "r");

	if (src_fd == NULL) {
		fprintf(stderr, "Failed to open \"%s\"\n", argv[1]);
		return -1;
	}
	
	printf("bits 64\nextern printf\nsection .text\nglobal main\nmain:\n\tpush rbp\n\tlea r8, [rel stack]\n\txor r9, r9\n\txor r10, r10\n");
	int skp = 0;
	int if_count = 0;
	int if_stack[256] = {0};
	unsigned char if_index = 255;

	char *word;
	while (word = get_next_word(src_fd)) {
		if (!word) {
			break;
		}


		if (word[0] >= '0' && word[0] <= '9') {
			printf("\tmov ax, %s\n\tmov [r8], ax\n\tadd r8, 16\n", word);
		} else if (word[0] == ':' && word[1] == 0) {
			char *label = get_next_word(src_fd);

			if (!label) {
				fprintf(stderr, "Unexpected EOF after \":\"\n", word);
				return -1;
			}

			printf("\tjmp skip_%d\n_word_%s:\n", ++skp, label);
		} else if (word[0] == ';' && word[1] == 0) {
			printf("\tret\nskip_%d:\n", skp);
		} else if (!strncmp(word, "if", 255)) {
			printf("\tsub r8, 16\n\tmov ax, [r8]\n\tcmp ax, byte 0\n\tje if_%d\n", if_count);
			if_stack[++if_index] = if_count++;
			if (if_index == 255) {
				fprintf(stderr, "Nested ifs with a depth of more than 255 are forbidden (and very silly).\n");
				return -1;
			}
		} else if (!strncmp(word, "then", 255)) {
			printf("if_%d:\n", if_stack[if_index--]);
		} else if (!strncmp(word, "do", 255)) {
			printf("\tpush r9\n\tpush r10\n\tsub r8, 16\n\tmov r9w, [r8]\n\tsub r9w, 1\n\tsub r8, 16\n\tmov r10w, [r8]\ndo_%d:\n\tadd r9w, 1\n\tcmp r9w, r10w\n\tje loop_%d\n", if_count, if_count);
			if_stack[++if_index] = if_count++;
		} else if (!strncmp(word, "loop", 255)) {
			int loop_index = if_stack[if_index--];
			printf("\tjmp do_%d\n\tloop_%d:\n\tpop r10\n\tpop r9\n", loop_index, loop_index);
		} else if (!strncmp(word, ".\"", 255)) {
			printf("\txor rax, rax\n\tlea rsi, [rel str_%d]\n\tlea rdi, [rel str_str]\n\tpush r10\n\tpush r9\n\tpush r8\n\tcall printf wrt ..plt\n\tpop r8\n\tpop r9\n\tpop r10\n", STRINGS_LEN);
			get_string(src_fd);
		} else {
			for (int i = 0; i < count(KEYWORDS); i++) {
				if (!strncmp(KEYWORDS[i].word, word, 255)) {
					printf("%s\n", KEYWORDS[i].compiled);
					goto word_found;
				}
			}
			printf("\tcall _word_%s\n", word);
word_found:
		}
	}
	printf("\tpop rbp\n\tret\n\nsection .bss\n\tstack resb 65536\n\nsection .data\n\tdot_str db '%%d', 0\n\temit_str db '%%c', 0\n\tcr_str db 10, 0\n\tstr_str db '%%s', 0\n");
	while (STRINGS) {
		printf("\tstr_%d: db '%s', 0\n", --STRINGS_LEN, STRINGS->str);
		free(STRINGS->str);
		stringlist_t *old_string = STRINGS;
		STRINGS = STRINGS->next;
		free(old_string);

	}

	if (if_index != 255) {
		fprintf(stderr, "Unexpected EOF in %d-level nested if/loops\n", if_index);
		return -1;
	}

	return 0;
}
