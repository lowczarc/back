#include "ast.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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

char *get_string(FILE *fd, char terminator) {
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
		if (c == terminator) {
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

	return buffer;
}

struct token {
	char *word;
	enum AST_Token token;
};

#define count(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))

struct token TOKENS[] = {
	{":", DEFINITION_START},
	{";", DEFINITION_END},
	{"if", CONTROL_IF},
	{"else", CONTROL_ELSE},
	{"then", CONTROL_THEN},
	{"do", CONTROL_DO},
	{"loop", CONTROL_LOOP},
	{"begin", CONTROL_BEGIN},
	{"until", CONTROL_UNTIL},
	{"again", CONTROL_AGAIN},
	{"variable", VARIABLE},
	{"swap", STACK_SWAP},
	{"drop", STACK_DROP},
	{"dup", STACK_DUP},
	{"over", STACK_OVER},
	{"rot", STACK_ROT},
	{"leave", LEAVE},
};

AST_Node *lex(FILE *src_file) {
	AST_Node *head = NULL;
	AST_Node **walker = &head;

	char *word;
	while ((word = get_next_word(src_file))) {
		if (!word) {
			break;
		}

		*walker = malloc(sizeof(AST_Node));
		(*walker)->next = NULL;

		for (size_t i = 0; i < count(TOKENS); i++) {
			if (!strcmp(TOKENS[i].word, word)) {
				(*walker)->token = TOKENS[i].token;
				goto word_found;
			}
		}

		if (word[0] >= '0' && word[0] <= '9') {
			(*walker)->token = NUMBER;
			(*walker)->data.nb = atoi(word);
			goto word_found;
		}

		char *str;
		if (!strcmp(word, ".\"")) {
			(*walker)->token = PRINT_STRING;
			str = get_string(src_file, '"');
		} else if (!strcmp(word, "\"")) {
			(*walker)->token = STRING;
			str = get_string(src_file, '"');
		} else {
			(*walker)->token = WORD;
			str = word;
		}
		(*walker)->data.str = malloc(strlen(str));
		strcpy((*walker)->data.str, str);

word_found:

		walker = &(*walker)->next;
	}

	return head;
}

