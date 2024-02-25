#include "generation.h"
#include <stdlib.h>

AST_Node *definition_extraction(AST_Node **ast) {
	AST_Node *definitions = NULL;

	while (*ast) {
		if ((*ast)->token == DEFINITION) {
			AST_Node *next = (*ast)->next;
			(*ast)->next = definitions;
			definitions = *ast;
			*ast = next;
		} else {
			ast = &(*ast)->next;
		}
	}
	return definitions;
}

void _inplace_string_extraction(String_List **dest, AST_Node *ast, int *label) {
	AST_Node **walker = &ast;

	while (*walker) {
		switch ((*walker)->token) {
			case DEFINITION:
				_inplace_string_extraction(dest, (*walker)->data.definition.body, label);
				break;

			case CONDITION:
				_inplace_string_extraction(dest, (*walker)->data.condition.when_true, label);
				_inplace_string_extraction(dest, (*walker)->data.condition.when_false, label);
				break;

			case LOOP_DO:
			case LOOP_UNTIL:
			case LOOP_AGAIN:
				_inplace_string_extraction(dest, (*walker)->data.loop.body, label);
				break;

			case STRING:
			case PRINT_STRING:
				String_List *new_string = malloc(sizeof(String_List));

				new_string->str = (*walker)->data.str;
				new_string->next = *dest;
				new_string->label = *label;
				(*walker)->generation.str_label = (*label)++;

				*dest = new_string;
				break;

			default:
		}
		walker = &(*walker)->next;
	}
}

String_List *string_extraction(AST_Node *ast) {
	String_List *ret = NULL;
	int label = 0;

	_inplace_string_extraction(&ret, ast, &label);

	return ret;
}

void _inplace_variable_extraction(Variable_List **dest, AST_Node *ast, int *label) {
	AST_Node **walker = &ast;

	while (*walker) {
		switch ((*walker)->token) {
			case DEFINITION:
				_inplace_variable_extraction(dest, (*walker)->data.definition.body, label);
				break;

			case CONDITION:
				_inplace_variable_extraction(dest, (*walker)->data.condition.when_true, label);
				_inplace_variable_extraction(dest, (*walker)->data.condition.when_false, label);
				break;

			case LOOP_DO:
			case LOOP_UNTIL:
			case LOOP_AGAIN:
				_inplace_variable_extraction(dest, (*walker)->data.loop.body, label);
				break;

			case VARIABLE:
				Variable_List *new_string = malloc(sizeof(Variable_List));

				new_string->str = (*walker)->data.str;
				new_string->next = *dest;
				new_string->label = *label;
				(*walker)->generation.variable_nb = (*label)++;

				*dest = new_string;
				break;

			default:
		}
		walker = &(*walker)->next;
	}
}

Variable_List *variable_extraction(AST_Node *ast) {
	Variable_List *ret = NULL;
	int label = 0;

	_inplace_variable_extraction(&ret, ast, &label);

	return ret;
}
