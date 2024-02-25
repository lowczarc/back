#include "generation.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: forthc [source.forth]\n\t\t");
		return -1;
	}

	FILE *src_fd = fopen(argv[1], "r");

	if (src_fd == NULL) {
		fprintf(stderr, "Failed to open \"%s\"\n", argv[1]);
		return -1;
	}
	
	AST_Node *ast = parse_to_ast(src_fd);

	dbg_ast(stderr, ast, 0);

	assembly_generation(stdout, ast);

	return 0;
}
