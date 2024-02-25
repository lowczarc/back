#include "ast.h"

struct token {
	char *word;
	enum AST_Token token;
};

#define count(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))

struct token TOKEN_STR[] = {
	{"DEFINITION", DEFINITION},
	{"DEFINITION_START", DEFINITION_START},
	{"DEFINITION_END", DEFINITION_END},
	{"CONDITION", CONDITION},
	{"CONTROL_IF", CONTROL_IF},
	{"CONTROL_ELSE", CONTROL_ELSE},
	{"CONTROL_THEN", CONTROL_THEN},
	{"LOOP_DO", LOOP_DO},
	{"LOOP_AGAIN", LOOP_AGAIN},
	{"LOOP_UNTIL", LOOP_UNTIL},
	{"CONTROL_DO", CONTROL_DO},
	{"CONTROL_LOOP", CONTROL_LOOP},
	{"CONTROL_BEGIN", CONTROL_BEGIN},
	{"CONTROL_UNTIL", CONTROL_UNTIL},
	{"CONTROL_AGAIN", CONTROL_AGAIN},
	{"STACK_SWAP", STACK_SWAP},
	{"STACK_DROP", STACK_DROP},
	{"STACK_DUP", STACK_DUP},
	{"STACK_OVER", STACK_OVER},
	{"STACK_ROT", STACK_ROT},
	{"LEAVE", LEAVE},
	{"VARIABLE", VARIABLE},
	{"STRING", STRING},
	{"PRINT_STRING", PRINT_STRING},
	{"NUMBER", NUMBER},
	{"WORD", WORD},
	{"COMMENT", COMMENT},
};

char *token_str(enum AST_Token token) {
	char *name = "UNKNOWN";
	for (size_t i = 0; i < count(TOKEN_STR); i++) {
		if (TOKEN_STR[i].token == token) {
			name = TOKEN_STR[i].word;
		}
	}
	return name;
}

void dbg_ast(FILE *output, AST_Node *ast, int depth) {
	AST_Node **walker = &ast;
	while (*walker) {
		char *name = token_str((*walker)->token);
		for (int i = 0; i < depth; i++) {
			fprintf(output, "\t");
		}
		fprintf(output, "[%02d] %s", (*walker)->token, name);
		switch ((*walker)->token) {
			case STRING:
			case PRINT_STRING:
			case WORD:
			case COMMENT:
			case DEFINITION:
			case VARIABLE:
				fprintf(output, " (%s)", (*walker)->data.str);
				break;

			case NUMBER:
				fprintf(output, " (%d)", (*walker)->data.nb);
				break;

			default:
		}
		fprintf(output, "\n");

		switch ((*walker)->token) {
			case DEFINITION:
				dbg_ast(output, (*walker)->data.definition.body, depth + 1);
				break;

			case CONDITION:
				dbg_ast(output, (*walker)->data.condition.when_true, depth + 1);
				if ((*walker)->data.condition.when_false) {
					for (int i = 0; i < depth; i++) {
						fprintf(output, "\t");
					}
					fprintf(output, "ELSE\n");
					dbg_ast(output, (*walker)->data.condition.when_false, depth + 1);
				}
				break;

			case LOOP_DO:
			case LOOP_UNTIL:
			case LOOP_AGAIN:
				dbg_ast(output, (*walker)->data.loop.body, depth + 1);
				break;

			default:
		}

		walker = &(*walker)->next;
	}
}
