#pragma once
#include "azure_1.h"

typedef struct REGISTER_USED {
  const char* key;
  int value;
} REGISTER_USED;

typedef struct STRING_MEMORY_ADDRESS {
  const char* key;
  uint value;
} STRING_MEMORY_ADDRESS;

typedef struct VARIABLE {
  uint rbp_start;
  uint rbp_end;
  int add;
  const char* name;
  const char* in_register;
  const char* str_size;
} VARIABLE;

typedef struct STACK_FRAME {
  uint rbp;
  VARIABLE** variables;
  uint variable_count;
  uint variable_capacity;
  REGISTER_USED* registers[7];
} STACK_FRAME;

typedef struct COMPILER {
  DYNAMIC_STR* code;
  DYNAMIC_STR* top_code;
  STACK_FRAME** stack_frames;
  uint stack_frame_count;
  uint stack_frame_capacity;
  AST_NODE_ARRAY* ast;

  char** loop_stack;
  uint loop_stack_count;
  uint loop_stack_capacity;

  char** defined_functions;
  uint defined_functions_count;
  uint defined_functions_capacity;

  STRING_MEMORY_ADDRESS** strings;
  uint strings_count;
  uint strings_capacity;
  
  int is_rax_in_use;
  uint loop_count;
  uint is_count;
} COMPILER;

extern const char* registers[];
const char* compile(AST_NODE_ARRAY* ast);
void compile_stmt(COMPILER* self, AST_NODE* node);