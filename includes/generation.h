#include "ast.h"

typedef struct String_List {
	char *str;
	int label;
	struct String_List *next;
} String_List;

typedef String_List Variable_List;

AST_Node *definition_extraction(AST_Node **ast);
String_List *string_extraction(AST_Node *ast);
Variable_List *variable_extraction(AST_Node *ast);

void assembly_generation(FILE *output_file, AST_Node *ast);
