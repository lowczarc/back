#include "ast.h"
#include <stdlib.h>

struct GTT_Result {
	enum AST_Token terminator;
	AST_Node *result;
};

struct GTT_Result grow_tree_terminator(AST_Node ***walker, enum AST_Token *terminator, size_t terminator_size) {
	AST_Node **next = *walker;
	AST_Node *result = **walker;
	
	while (**walker) {
		for (size_t i = 0; i < terminator_size; i++) {
			if ((**walker)->token == terminator[i]) {
				AST_Node *to_free = **walker;
				if (result == to_free) {
					result = NULL;
				}
				**walker = NULL;
				*next = to_free->next;
				*walker = next;
				free(to_free);
				struct GTT_Result r = {terminator[i], result};
				return r;
			}
		}

		AST_Node *current_node;
		struct GTT_Result r;
		switch ((**walker)->token) {
			case DEFINITION_START:
				(**walker)->token = DEFINITION;

				current_node = (**walker);

				*walker = &(**walker)->next;

				if ((**walker)->token != WORD) {
					fprintf(stderr, "Unexpected token [%s] after DEFINITION_START. Expected WORD.\n", token_str((**walker)->token));
					exit(-1);
				}

				current_node->data.definition.name = (**walker)->data.str;

				*walker = &(**walker)->next;

				enum AST_Token definition_terminator[1] = {DEFINITION_END};

				r = grow_tree_terminator(walker, definition_terminator, 1);
				current_node->data.definition.body = r.result;

				current_node->next = **walker;
				break;

			case CONTROL_IF:
				(**walker)->token = CONDITION;

				current_node = (**walker);

				*walker = &(**walker)->next;

				enum AST_Token condition_terminator[2] = {CONTROL_THEN, CONTROL_ELSE};

				r = grow_tree_terminator(walker, condition_terminator, 2);
				current_node->data.condition.when_true = r.result;

				if (r.terminator == CONTROL_ELSE) {
					r = grow_tree_terminator(walker, condition_terminator, 1);
					current_node->data.condition.when_false = r.result;
				}

				current_node->next = **walker;
				break;

			case CONTROL_DO:
				(**walker)->token = LOOP_DO;

				current_node = (**walker);

				*walker = &(**walker)->next;

				enum AST_Token loop_do_terminator[1] = {CONTROL_LOOP};

				r = grow_tree_terminator(walker, loop_do_terminator, 1);
				current_node->data.loop.body = r.result;

				current_node->next = **walker;
				break;

			case CONTROL_BEGIN:
				current_node = (**walker);

				*walker = &(**walker)->next;

				enum AST_Token loop_begin_terminator[2] = {CONTROL_UNTIL, CONTROL_AGAIN};

				r = grow_tree_terminator(walker, loop_begin_terminator, 2);
				current_node->data.loop.body = r.result;

				if (r.terminator == CONTROL_UNTIL) {
					current_node->token = LOOP_UNTIL;
				} else if (r.terminator == CONTROL_AGAIN) {
					current_node->token = LOOP_AGAIN;
				}

				current_node->next = **walker;
				break;

			case VARIABLE:
				AST_Node *next_node = (**walker)->next;

				if (next_node->token != WORD) {
					fprintf(stderr, "Unexpected token [%s] after VARIABLE. Expected WORD.\n", token_str((**walker)->token));
					exit(-1);
				}

				(**walker)->data.str = next_node->data.str;
				(**walker)->next = next_node->next;
				free(next_node);

				*walker = &(**walker)->next;
				break;

			default:
				*walker = &(**walker)->next;
		}
	}

	if (terminator_size != 0) {
		fprintf(stderr, "Unexpected EOF. Expected ");
		for (size_t i = 0; i < terminator_size; i++) {
			fprintf(stderr, "%s%s", i == 0 ? "" : i == terminator_size - 1 ? " or " : ", ", token_str(terminator[i]));
		}
		fprintf(stderr, ".\n");
		exit(-1);
	}
	struct GTT_Result r = {NONE, result};
	return r;
}

AST_Node *grow_tree(AST_Node ***walker) {
	struct GTT_Result r = grow_tree_terminator(walker, NULL, 0);
	return r.result;
}
