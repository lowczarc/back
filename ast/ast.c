#include "ast.h"

AST_Node *parse_to_ast(FILE *src_file) {
	AST_Node *tokenized = lex(src_file);
	AST_Node **walker = &tokenized;
	AST_Node *result = grow_tree(&walker);

	verify_ast(result, 1);

	return result;
}
