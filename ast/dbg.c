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
	{"VARIABLE", VARIABLE},
	{"STRING", STRING},
	{"PRINT_STRING", PRINT_STRING},
	{"NUMBER", NUMBER},
	{"WORD", WORD},
	{"COMMENT", COMMENT},
};

void dbg_ast(AST_Node *ast, int depth) {
	AST_Node **walker = &ast;
	while (*walker) {
		char *name = "UNKNOWN";
		for (size_t i = 0; i < count(TOKEN_STR); i++) {
			if (TOKEN_STR[i].token == (*walker)->token) {
				name = TOKEN_STR[i].word;
			}
		}
		for (int i = 0; i < depth; i++) {
			printf("\t");
		}
		printf("[%02d] %s", (*walker)->token, name);
		if ((*walker)->token == STRING || (*walker)->token == PRINT_STRING
 			|| (*walker)->token == WORD || (*walker)->token == COMMENT) {
			printf(" (%s)", (*walker)->data.str);
		}
		if ((*walker)->token == NUMBER) {
			printf(" (%d)", (*walker)->data.nb);
		}
		printf("\n");
		walker = &(*walker)->next;
	}
}
