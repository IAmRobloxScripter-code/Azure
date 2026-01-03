#include <stdio.h>

#include "azure_2.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "No file provided!\n");
    return 1;
  }
  FILE* file = fopen(argv[1], "rb");
  if (!file) {
    fprintf(stderr, "Could not open file!\n");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long content_size = ftell(file);
  rewind(file);

  char* content = malloc(content_size + 1);
  if (!content) {
    fprintf(stderr, "Failed to allocate file content buffer!\n");
    fclose(file);
    return 1;
  }

  fread(content, 1, content_size, file);
  fclose(file);

  LEX_TOKEN_ARRAY* tokens = lex_input_str(content);
  AST_NODE_ARRAY* ast = parse_tokens(tokens, argv[1]);
  const char* assembly = compile(ast);
  const char* out_name = "a.s";

  if (argc > 3 && strcmp(argv[2], "-o") == 0) {
    out_name = argv[3];
  }

  FILE* out_file = fopen(out_name, "w");

  if (out_file == NULL) {
    perror("Error opening file");
    return 1;
  }

  fprintf(out_file, "%s", assembly);
  fclose(out_file);
}