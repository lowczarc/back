#include "generation.h"
#include <string.h>

#define PUSH(X) "\tmov [r8], " #X "\n\tadd r8, 8\n"
#define POP(X) "\tsub r8, 8\n\tmov " #X ", [r8]\n"
#define SAVE_REGS(X) "\tpush r8\n\tpush r9\n\tpush r10\n\tpush r12\n" X "\tpop r12\n\tpop r10\n\tpop r9\n\tpop r8\n"

void asm_strings(FILE *output, String_List *list) {
	String_List **walker = &list;

	fprintf(output, "\
dot_str: db '%%lld', 0\n\
hex_str: db '0x%%016llx', 0\n\
str_str: db '%%s', 0\n\
cr_str: db 10, 0\n\
emit_str: db '%%c', 0\n");
	while (*walker) {
		fprintf(output, "str_%d: db '%s', 0\n", (*walker)->label, (*walker)->str);
		walker = &(*walker)->next;
	}
}

struct keyword {
	char *word;
	char *compiled;
};

#define count(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))

struct keyword KEYWORDS[] = {
	{ "+", POP(rbx) POP(rax) "\tadd rax, rbx\n" PUSH(rax) },
	{ "-", POP(rbx) POP(rax) "\tsub rax, rbx\n" PUSH(rax) },
	{ "*", POP(rbx) POP(rax) "\tmul rbx\n" PUSH(rax) },
	{ "/", POP(rbx) POP(rax) "\txor rdx, rdx\n\tdiv rbx\n" PUSH(rax) },
	{ "mod", POP(rbx) POP(rax) "\txor rdx, rdx\n\tdiv rbx\n" PUSH(rdx) },
	{ "/mod", POP(rbx) POP(rax) "\txor rdx, rdx\n\tdiv rbx\n" PUSH(rdx) PUSH(rax) },
	{ "and", POP(rbx) POP(rax) "\tand rax, rbx\n" PUSH(rax) },
	{ "or", POP(rbx) POP(rax) "\tor rax, rbx\n" PUSH(rax) },
	{ "xor", POP(rbx) POP(rax) "\txor rax, rbx\n" PUSH(rax) },
	{ "not", POP(rax) "\txor rbx, rbx\n\txor rax, byte 0\n\tsetz bl\n" PUSH(rbx) },
	{ "=", POP(rbx) POP(rax) "\txor rdx, rdx\n\tcmp eax, ebx\n\tsetz dl\n" PUSH(rdx) },
	{ ">", POP(rbx) POP(rax) "\txor rdx, rdx\n\tcmp eax, ebx\n\tsetg dl\n" PUSH(rdx) },
	{ "<", POP(rbx) POP(rax) "\txor rdx, rdx\n\tcmp ebx, eax\n\tsetg dl\n" PUSH(rdx) },
	{ ".", POP(rsi) SAVE_REGS("\txor rax, rax\n\tlea rdi, [rel dot_str]\n\tcall printf wrt ..plt\n") },
	{ ".x", POP(rsi) SAVE_REGS("\txor rax, rax\n\tlea rdi, [rel hex_str]\n\tcall printf wrt ..plt\n") },
	{ "key", SAVE_REGS("\txor rax, rax\n\tcall getchar\n") PUSH(rax) },
	{ "emit", POP(rsi) SAVE_REGS("\txor rax, rax\n\tlea rdi, [rel emit_str]\n\tcall printf wrt ..plt\n") },
	{ "cr", SAVE_REGS("\txor rax, rax\n\tlea rdi, [rel cr_str]\n\tcall printf wrt ..plt\n") },
	{ "i", PUSH(r9) },
	{ "alot", POP(rax) "\tadd r11, rax" },
	{ "!", POP(rax) POP(rbx) "\tmov [rax], rbx" },
	{ "@", "\n; @\n" POP(rax) "\tmov rax, [rax]\n" PUSH(rax) "; DUP\n\n" },
	{ "cells", POP(rax) "\tmov rbx, 4\n\tmul rbx\n" PUSH(rax) },
};

void asm_ast(FILE *output, AST_Node *ast, int current_loop) {
	AST_Node **walker = &ast;
	static int counter = 0;

	int ctrl_id;
	while (*walker) {
		switch ((*walker)->token) {
			case DEFINITION:
				fprintf(output, "_word_%s:\n", (*walker)->data.definition.name);
				asm_ast(output, (*walker)->data.definition.body, current_loop);
				fprintf(output, "\tret\n");
				break;

			case LOOP_DO:
				ctrl_id = counter++;
				fprintf(output, "\
	push r9\n\
	push r10\n\
" POP(r9) POP(r10) "\
_loop_start_%d:\n\
	cmp r9, r10\n\
	je _loop_end_%d\n", ctrl_id, ctrl_id);
				asm_ast(output, (*walker)->data.loop.body, ctrl_id);
				fprintf(output, "\
	add r9, 1\n\
	jmp _loop_start_%d\n\
_loop_end_%d:\
	pop r10\n\
	pop r9\n", ctrl_id, ctrl_id);
				break;

			case LOOP_AGAIN:
				ctrl_id = counter++;
				fprintf(output, "_loop_start_%d:\n", ctrl_id);
				asm_ast(output, (*walker)->data.loop.body, ctrl_id);
				fprintf(output, "\
	jmp _loop_start_%d\n\
_loop_end_%d:\n", ctrl_id, ctrl_id);
				break;

			case CONDITION:
				ctrl_id = counter++;
				fprintf(output, POP(rax) "\
	cmp rax, byte 0\n\
	je _condition_else_%d\n", ctrl_id);
				asm_ast(output, (*walker)->data.condition.when_true, current_loop);
				fprintf(output, "\
	jmp _condition_end_%d\n\
_condition_else_%d:\n", ctrl_id, ctrl_id);
				asm_ast(output, (*walker)->data.condition.when_false, current_loop);
				fprintf(output, "_condition_end_%d:\n", ctrl_id);
				break;

			case LEAVE:
				fprintf(output, "\tjmp _loop_end_%d\n", current_loop);
				break;

			case NUMBER:
				fprintf(output, "\tmov rax, %d\n" PUSH(rax), (*walker)->data.nb);
				break;

			case STRING:
				fprintf(output, "\tmov rax, str_%d\n" PUSH(rax), (*walker)->generation.str_label);
				break;

			case PRINT_STRING:
				fprintf(output, SAVE_REGS("\
	xor rax, rax\n\
	lea rsi, [rel str_%d]\n\
	lea rdi, [rel str_str]\n\
	call printf wrt ..plt\n"), (*walker)->generation.str_label);
				break;

			case STACK_SWAP:
				fprintf(output, "\n; SWAP\n");
				fprintf(output, POP(rax) POP(rbx) PUSH(rax) PUSH(rbx));
				fprintf(output, "; END SWAP\n\n");
				break;

			case STACK_DROP:
				fprintf(output, POP(rax));
				break;

			case STACK_DUP:
				fprintf(output, "\n; DUP\n");
				fprintf(output, POP(rax) PUSH(rax) PUSH(rax));
				fprintf(output, "; END DUP\n\n");
				break;

			case STACK_OVER:
				fprintf(output, POP(rax) POP(rbx) PUSH(rbx) PUSH(rax) PUSH(rbx));
				break;

			case STACK_ROT:
				fprintf(output, POP(rax) POP(rbx) POP(rdx) PUSH(rbx) PUSH(rax) PUSH(rdx));
				break;

			case WORD:
			  for (size_t i = 0; i < count(KEYWORDS); i++) {
					if (!strncmp(KEYWORDS[i].word, (*walker)->data.str, 255)) {
						printf("%s", KEYWORDS[i].compiled);
						goto word_found;
					}
				}

				printf("\tcall _word_%s\n", (*walker)->data.str);

word_found:
				break;


			default:
				fprintf(stderr, "Unimplemented token asm conversion [%s]\n", token_str((*walker)->token));
		}
		walker = &(*walker)->next;
	}
}

void assembly_generation(FILE *output, AST_Node *ast) {
	String_List *str = string_extraction(ast);

	AST_Node *definitions = definition_extraction(&ast);

	fprintf(output,
"bits 64\n\
extern printf\n\
extern getchar\n\n\
section .text\n\
global main\n\n\
main:\n\
	push rbp\n\
	lea r8, [rel stack]\n\
	xor r9, r9\n\
	xor r10, r10\n\
	xor r12, r12\n");

	asm_ast(output, ast, -1);

	fprintf(output,
"	pop rbp\n\
	ret\n\n");

	asm_ast(output, definitions, -1);

	fprintf(output,
"section .bss\n\
stack resb 65536\n\n\
section .data\n");

	asm_strings(output, str);
}
