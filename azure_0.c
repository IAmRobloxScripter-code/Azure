#include <stdio.h>

#include "azure_2.h"

int main(int argc, char* argv[]) {  // this was made by claude cuz idk how
  if (argc < 2) {
    fprintf(stderr,
            "Usage: %s <input_file> [-o output] [-l file1.o file2.o ...]\n",
            argv[0]);
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
  content[content_size] = '\0';
  fclose(file);

  LEX_TOKEN_ARRAY* tokens = lex_input_str(content);
  AST_NODE_ARRAY* ast = parse_tokens(tokens, argv[1]);
  const char* assembly = compile(ast);

  // printf("%s\n", assembly);

  char* exe_name = "a.out";
  char link_files[1024] = "";

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      exe_name = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "-l") == 0) {
      for (int j = i + 1; j < argc; j++) {
        if (argv[j][0] == '-') break;
        strcat(link_files, argv[j]);
        strcat(link_files, " ");
        i = j;
      }
    }
  }

  char asm_file[512];
  snprintf(asm_file, sizeof(asm_file), "/tmp/%s_temp.s", exe_name);

  FILE* out_file = fopen(asm_file, "w");
  if (out_file == NULL) {
    perror("Error creating assembly file");
    free(content);
    return 1;
  }

  fprintf(out_file, "%s", assembly);
  fclose(out_file);

  char obj_file[512];
  snprintf(obj_file, sizeof(obj_file), "/tmp/%s_temp.o", exe_name);

  char nasm_cmd[1024];
  snprintf(nasm_cmd, sizeof(nasm_cmd), "nasm -f elf64 %s -o %s", asm_file,
           obj_file);

  if (system(nasm_cmd) != 0) {
    fprintf(stderr, "Assembly failed\n");
    remove(asm_file);
    free(content);
    return 1;
  }

  char ld_cmd[2048];
  if (strlen(link_files) > 0) {
    snprintf(ld_cmd, sizeof(ld_cmd), "ld %s %s -o %s", obj_file, link_files,
             exe_name);
  } else {
    snprintf(ld_cmd, sizeof(ld_cmd), "ld %s -o %s", obj_file, exe_name);
  }

  if (system(ld_cmd) != 0) {
    fprintf(stderr, "Linking failed\n");
    remove(asm_file);
    remove(obj_file);
    free(content);
    return 1;
  }

  remove(asm_file);
  remove(obj_file);

  printf("Successfully created: %s\n", exe_name);

  free(content);
  return 0;
}