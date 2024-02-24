#include <stdio.h>

enum AST_Token {
	DEFINITION,
	DEFINITION_START,
	DEFINITION_END,

	CONDITION,
	CONTROL_IF,
	CONTROL_ELSE,
	CONTROL_THEN,

	CONTROL_DO,
	CONTROL_LOOP,

	CONTROL_BEGIN,
	CONTROL_UNTIL,
	CONTROL_AGAIN,

	STACK_SWAP,
	STACK_DROP,
	STACK_DUP,
	STACK_OVER,
	STACK_ROT,

	VARIABLE,

	STRING,
	PRINT_STRING,

	NUMBER,

	WORD,

	COMMENT,
};

union AST_Node_Data {
	char *str;
	int nb;
	struct AST_Node *definition;
};

typedef struct AST_Node {
	enum AST_Token token;
	union AST_Node_Data data;
	struct AST_Node *next;
} AST_Node;

AST_Node *parse_to_ast(FILE *src_file);
void dbg_ast(AST_Node *ast, int depth);
