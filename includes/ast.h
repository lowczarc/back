#include <stdio.h>

enum AST_Token {
	NONE,

	DEFINITION,
	DEFINITION_START,
	DEFINITION_END,

	CONDITION,
	CONTROL_IF,
	CONTROL_ELSE,
	CONTROL_THEN,

	LOOP_DO,
	CONTROL_DO,
	CONTROL_LOOP,

	LOOP_UNTIL,
	LOOP_AGAIN,
	CONTROL_BEGIN,
	CONTROL_UNTIL,
	CONTROL_AGAIN,

	LEAVE,

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

	_TOKEN_MAX,
};

struct AST_Node_Definition {
		char *name;
		struct AST_Node *body;
};

struct AST_Node_Loop {
		struct AST_Node *body;
};

struct AST_Node_Condition{
		char *name;
		struct AST_Node *when_true;
		struct AST_Node *when_false;
};

union AST_Node_Data {
	char *str;
	int nb;
	struct AST_Node_Definition definition;
	struct AST_Node_Condition condition;
	struct AST_Node_Loop loop;
};

typedef struct AST_Node {
	enum AST_Token token;
	union AST_Node_Data data;
	struct AST_Node *next;
} AST_Node;

AST_Node *parse_to_ast(FILE *src_file);

void dbg_ast(AST_Node *ast, int depth);
char *token_str(enum AST_Token token);

void verify_ast(AST_Node *ast, int root);

AST_Node *grow_tree(AST_Node ***walker);
AST_Node *lex(FILE *src_file);
