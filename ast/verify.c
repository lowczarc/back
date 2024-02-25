#include "ast.h"
#include <stdio.h>
#include <unistd.h>

void verify_ast(AST_Node *ast, int root) {
	AST_Node **walker = &ast;

	while (*walker) {
		if ((*walker)->token <= NONE || (*walker)->token >= _TOKEN_MAX) {
			fprintf(stderr, "FATAL ERROR: Unknown token [%d] found in AST.\n\tThis should never happen and is probably a bug in the compiler.\n\tPlease report the issue along with the command you tried to execute and a forth source file where it happened. (https://github.com/lowczarc/back/issues)", (*walker)->token);
			_exit(-1);
		}

		switch ((*walker)->token) {
			case DEFINITION:
				if (!root) {
					fprintf(stderr, "Unexpected DEFINITION (word: %s) outside of top-level. DEFINTION should only be defined at the top level.", (*walker)->data.str);
					_exit(-1);
				}
				verify_ast((*walker)->data.definition.body, 0);
				break;

			case CONDITION:
				verify_ast((*walker)->data.condition.when_true, 0);
				verify_ast((*walker)->data.condition.when_false, 0);
				break;

			case LOOP_DO:
			case LOOP_UNTIL:
			case LOOP_AGAIN:
				verify_ast((*walker)->data.loop.body, 0);
				break;

			case CONTROL_IF:
			case CONTROL_DO:
			case CONTROL_BEGIN:
			case DEFINITION_START:
				fprintf(stderr, "FATAL ERROR: Starting control token [%s] found after the AST building step.\n\tThis should never happen and is probably a bug in the compiler.\n\tPlease report the issue along with the command you tried to execute and a forth source file where it happened. (https://github.com/lowczarc/back/issues)", token_str((*walker)->token));
				_exit(-1);

			case CONTROL_THEN:
			case CONTROL_LOOP:
			case CONTROL_UNTIL:
			case CONTROL_AGAIN:
			case DEFINITION_END:
				fprintf(stderr, "Unexpected closing control token [%s]", token_str((*walker)->token));
				_exit(-1);

			default:
		}
		walker = &(*walker)->next;
	}
}
