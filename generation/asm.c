#include "generation.h"

void asm_strings(FILE *output, String_List *list) {
	String_List **walker = &list;

	while (*walker) {
		fprintf(output, "str_%d: db '%s', 0\n", (*walker)->label, (*walker)->str);
		walker = &(*walker)->next;
	}
}

void assembly_generation(FILE *output, AST_Node *ast) {
	String_List *str = string_extraction(ast);

	asm_strings(output, str);
}
