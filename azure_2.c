#include "azure_2.h"

#include <stdio.h>

#define SIZEOF_PTR __SIZEOF_POINTER__
#define SIZEOF_INT 8
#define SIZEOF_CHAR 1

const char* registers[] = {"r8", "r9", "r10", "r11"};

int find_const_string(COMPILER* self, const char* value) {
  for (uint index = 0; index < self->strings_count; ++index) {
    if (self->strings[index]->key == value) return self->strings[index]->value;
  }

  return -1;
};

const char* to_string(int value) {
  int temp = value;
  int digits = 0;
  do {
    digits += 1;
    temp /= 10;
  } while (temp > 0);

  char* string = malloc(digits + 1);

  string[digits] = '\0';
  int i = digits - 1;

  do {
    string[i--] = (value % 10) + '0';
    value /= 10;
  } while (value > 0);

  return string;
}

REGISTER_USED* get_unused_register(STACK_FRAME* stack_frame) {
  for (int index = 0; index < 4; ++index) {
    if (stack_frame->registers[index]->value == 0)
      return stack_frame->registers[index];
  }
  return 0;
}

uint get_type_offset(AST_NODE_TYPE_DECL* node) {
  switch (node->type_kind) {
    case BASE_TYPE:
      if (strcmp(node->base_type, "int") == 0) return SIZEOF_INT;
      if (strcmp(node->base_type, "char") == 0) return SIZEOF_CHAR;
      return SIZEOF_PTR;
    case POINTER_TYPE:
      return SIZEOF_PTR;
    case ARRAY_TYPE:
      return SIZEOF_PTR;
    default:
      return 0;
  }
}

const char* get_type_offset_name(AST_NODE_TYPE_DECL* node) {
  switch (node->type_kind) {
    case BASE_TYPE:
      if (strcmp(node->base_type, "int") == 0) return "QWORD";
      if (strcmp(node->base_type, "char") == 0) return "BYTE";
      return "QWORD";
    case POINTER_TYPE:
      return "QWORD";
    case ARRAY_TYPE:
      return "QWORD";
    default:
      return NULL;
  }
}

int is_global_function(COMPILER* self, const char* name) {
  for (uint index = 0; index < self->defined_functions_count; ++index) {
    if (strcmp(self->defined_functions[index], name) == 0) return 1;
  }
  return 0;
}

void compile_fn_decl(COMPILER* self, AST_NODE_FN_DECL* node) {
  APPEND_DYNAMIC_STR(self->code, node->name);
  APPEND_DYNAMIC_STR(self->code, ":\npush rbp\nmov rbp, rsp\n");
  DYNAMIC_STR* last_code = self->code;
  self->code = malloc(sizeof(DYNAMIC_STR));
  self->code->count = 0;
  self->code->capacity = 16;
  self->code->value = malloc(sizeof(char*) * self->code->capacity);

  if (self->defined_functions_count >= self->defined_functions_capacity) {
    self->defined_functions_capacity *= 2;
    self->defined_functions =
        realloc(self->defined_functions,
                sizeof(char*) * self->defined_functions_capacity);
  }

  self->defined_functions[self->defined_functions_count++] = (char*)node->name;

  STACK_FRAME* stack_frame = malloc(sizeof(STACK_FRAME));
  stack_frame->rbp = 8;
  stack_frame->variable_count = 0;
  stack_frame->variable_capacity = 16;
  stack_frame->variables =
      malloc(sizeof(VARIABLE*) * stack_frame->variable_capacity);

  if (self->stack_frame_count >= self->stack_frame_capacity) {
    self->stack_frame_capacity *= 2;
    self->stack_frames = realloc(
        self->stack_frames, sizeof(STACK_FRAME*) * self->stack_frame_capacity);
  }

  self->stack_frames[self->stack_frame_count++] = stack_frame;

  for (uint index = 0; index < 4; ++index) {
    stack_frame->registers[index] = malloc(sizeof(REGISTER_USED));
    stack_frame->registers[index]->key = registers[index];
    stack_frame->registers[index]->value = 0;
  }

  for (uint index = 0; index < node->param_count; ++index) {
    AST_NODE_PARAM* param = node->params[index];

    VARIABLE* variable = malloc(sizeof(VARIABLE));
    variable->name = param->name;
    variable->str_size = get_type_offset_name(param->type);
    variable->add = 1;

    if (stack_frame->variable_count >= stack_frame->variable_capacity) {
      stack_frame->variable_capacity *= 2;
      stack_frame->variables =
          realloc(stack_frame->variables,
                  sizeof(VARIABLE*) * stack_frame->variable_capacity);
    }

    stack_frame->variables[stack_frame->variable_count++] = variable;

    variable->rbp_start = stack_frame->rbp;
    stack_frame->rbp += get_type_offset(param->type);
    variable->rbp_end = stack_frame->rbp;
  }

  for (uint index = 0; index < node->body->count; ++index) {
    compile_stmt(self, node->body->array[index]);
  }

  if (node->body->array[node->body->count - 1]->kind != RETURN_EXPR) {
    APPEND_DYNAMIC_STR(self->code, "add rsp, ");
    APPEND_DYNAMIC_STR(self->code, to_string(8 + stack_frame->rbp));
    APPEND_DYNAMIC_STR(self->code, "\npop rbp\nmov rax, 0\nret\n");
  }

  APPEND_DYNAMIC_STR(last_code, "sub rsp, ");
  APPEND_DYNAMIC_STR(last_code, to_string(8 + stack_frame->rbp));
  APPEND_DYNAMIC_STR(last_code, "\n");
  APPEND_DYNAMIC_STR(last_code, self->code->value);

  free(self->code);
  self->code = last_code;
  self->stack_frame_count -= 1;
  free(stack_frame->variables);
  free(stack_frame);
  free(node->return_type);
  free(node->body);
  free(node);
}

VARIABLE* find_variable(STACK_FRAME* stack_frame, const char* name) {
  for (uint index = 0; index < stack_frame->variable_count; ++index) {
    if (strcmp(stack_frame->variables[index]->name, name) == 0)
      return stack_frame->variables[index];
  }
  fprintf(stderr, "Variable \"%s\" does not exist!\n", name);
  exit(1);
  return 0;
}

void string_make(COMPILER* self, AST_NODE* node) {
  if (find_const_string(self, node->VALUE_STRING_LITERAL->value) == -1) {
    APPEND_DYNAMIC_STR(self->top_code, "LC");
    APPEND_DYNAMIC_STR(self->top_code, to_string(self->strings_count));
    APPEND_DYNAMIC_STR(self->top_code, ":\n");
    APPEND_DYNAMIC_STR(self->top_code, "db ")
    int in_string_bool = 0;

    for (uint index = 0; index < strlen(node->VALUE_STRING_LITERAL->value);
         ++index) {
      char c = node->VALUE_STRING_LITERAL->value[index];
      if (c == '\0' || c == '\n' || c == '\"' || c == '\'' || c == '\\' ||
          c == '\r' || c == '\t') {
        if (in_string_bool == 1) {
          APPEND_DYNAMIC_STR(self->top_code, "\"");
          in_string_bool = 0;
        }

        APPEND_DYNAMIC_STR(self->top_code, ", ");
        APPEND_DYNAMIC_STR(self->top_code, to_string((int)c));
        if (index != strlen(node->VALUE_STRING_LITERAL->value) - 1) {
          APPEND_DYNAMIC_STR(self->top_code, ", ");
        }
      } else {
        if (in_string_bool == 0) {
          APPEND_DYNAMIC_STR(self->top_code, "\"");
          in_string_bool = 1;
        }
        char* ch = malloc(2);
        ch[0] = c;
        ch[1] = '\0';
        APPEND_DYNAMIC_STR(self->top_code, ch);
        free(ch);
      }
    }
    if (in_string_bool == 1) {
      APPEND_DYNAMIC_STR(self->top_code, "\"");
      in_string_bool = 0;
    }
    APPEND_DYNAMIC_STR(self->top_code, ", 0");
    APPEND_DYNAMIC_STR(self->top_code, "\n");

    if (self->strings_count >= self->strings_capacity) {
      self->strings_capacity *= 2;
      self->strings = realloc(self->strings, sizeof(STRING_MEMORY_ADDRESS*) *
                                                 self->strings_capacity);
    }

    STRING_MEMORY_ADDRESS* string_memory_address =
        malloc(sizeof(STRING_MEMORY_ADDRESS));
    string_memory_address->key = node->VALUE_STRING_LITERAL->value;
    string_memory_address->value = self->strings_count;
    self->strings[self->strings_count++] = string_memory_address;
  }
}

void compile_is_stmt(COMPILER* self, AST_NODE_IS_STMT* node) {
  self->is_count++;

  DYNAMIC_STR* label_name = malloc(sizeof(DYNAMIC_STR));
  label_name->count = 0;
  label_name->capacity = 16;
  label_name->value = malloc(sizeof(char) * label_name->capacity);

  APPEND_DYNAMIC_STR(label_name, "IS");
  APPEND_DYNAMIC_STR(label_name, to_string(self->is_count - 1));

  if (node->condition->kind == IDENTIFIER_LITERAL) {
    compile_stmt(self, node->condition);
    APPEND_DYNAMIC_STR(self->code, "cmp rax, 0\n");
    APPEND_DYNAMIC_STR(self->code, "jne ");
    APPEND_DYNAMIC_STR(self->code, label_name->value);
    APPEND_DYNAMIC_STR(self->code, "\n");
    if (node->else_body != 0) {
      APPEND_DYNAMIC_STR(self->code, "jmp ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
      APPEND_DYNAMIC_STR(self->code, "_ELSE\n");
    } else {
      APPEND_DYNAMIC_STR(self->code, "jmp ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
      APPEND_DYNAMIC_STR(self->code, "_END\n");
    }
    self->is_rax_in_use = 0;
  } else if (node->condition->kind == BINARY_EXPR) {
    self->is_rax_in_use = 0;
    compile_stmt(self, node->condition->BINARY_EXPR->left);
    APPEND_DYNAMIC_STR(self->code, "push rax\n");
    compile_stmt(self, node->condition->BINARY_EXPR->right);
    APPEND_DYNAMIC_STR(self->code, "push r8\n");

    APPEND_DYNAMIC_STR(self->code, "pop r8\n");
    APPEND_DYNAMIC_STR(self->code, "pop rax\n");

    APPEND_DYNAMIC_STR(self->code, "cmp rax, r8\n");

    if (strcmp(node->condition->BINARY_EXPR->operator, "==") == 0) {
      APPEND_DYNAMIC_STR(self->code, "je ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
    } else if (strcmp(node->condition->BINARY_EXPR->operator, "!=") == 0) {
      APPEND_DYNAMIC_STR(self->code, "jne ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
    } else if (strcmp(node->condition->BINARY_EXPR->operator, ">") == 0) {
      APPEND_DYNAMIC_STR(self->code, "jg ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
    } else if (strcmp(node->condition->BINARY_EXPR->operator, "<") == 0) {
      APPEND_DYNAMIC_STR(self->code, "jl ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
    } else if (strcmp(node->condition->BINARY_EXPR->operator, ">=") == 0) {
      APPEND_DYNAMIC_STR(self->code, "jge ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
    } else if (strcmp(node->condition->BINARY_EXPR->operator, "<=") == 0) {
      APPEND_DYNAMIC_STR(self->code, "jle ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
    }

    APPEND_DYNAMIC_STR(self->code, "\n");
    if (node->else_body != 0) {
      APPEND_DYNAMIC_STR(self->code, "jmp ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
      APPEND_DYNAMIC_STR(self->code, "_ELSE\n");
    } else {
      APPEND_DYNAMIC_STR(self->code, "jmp ");
      APPEND_DYNAMIC_STR(self->code, label_name->value);
      APPEND_DYNAMIC_STR(self->code, "_END\n");
    }
    self->is_rax_in_use = 0;
  }

  APPEND_DYNAMIC_STR(self->code, label_name->value);
  APPEND_DYNAMIC_STR(self->code, ":\n");

  for (uint index = 0; index < node->body->count; ++index) {
    compile_stmt(self, node->body->array[index]);
  }

  APPEND_DYNAMIC_STR(self->code, "jmp ");
  APPEND_DYNAMIC_STR(self->code, label_name->value);
  APPEND_DYNAMIC_STR(self->code, "_END\n");

  if (node->else_body != 0) {
    APPEND_DYNAMIC_STR(self->code, label_name->value);
    APPEND_DYNAMIC_STR(self->code, "_ELSE:\n");

    for (uint index = 0; index < node->else_body->count; ++index) {
      compile_stmt(self, node->else_body->array[index]);
    }

    APPEND_DYNAMIC_STR(self->code, "jmp ");
    APPEND_DYNAMIC_STR(self->code, label_name->value);
    APPEND_DYNAMIC_STR(self->code, "_END\n");
  }

  APPEND_DYNAMIC_STR(self->code, label_name->value);
  APPEND_DYNAMIC_STR(self->code, "_END:\n");
}

void compile_break_stmt(COMPILER* self, AST_NODE* node) {
  APPEND_DYNAMIC_STR(self->code, "jmp ");
  APPEND_DYNAMIC_STR(self->code, self->loop_stack[self->loop_stack_count - 1]);
  APPEND_DYNAMIC_STR(self->code, "_END\n");
  free(node);
}

void compile_continue_stmt(COMPILER* self, AST_NODE* node) {
  APPEND_DYNAMIC_STR(self->code, "jmp ");
  APPEND_DYNAMIC_STR(self->code, self->loop_stack[self->loop_stack_count - 1]);
  APPEND_DYNAMIC_STR(self->code, "\n");
  free(node);
}

void compile_loop_stmt(COMPILER* self, AST_NODE_LOOP_STMT* node) {
  self->loop_count++;
  DYNAMIC_STR* label_name = malloc(sizeof(DYNAMIC_STR));
  label_name->count = 0;
  label_name->capacity = 16;
  label_name->value = malloc(sizeof(char) * label_name->capacity);

  if (self->loop_stack_count >= self->loop_stack_capacity) {
    self->loop_stack_capacity *= 2;
    self->loop_stack =
        realloc(self->loop_stack, sizeof(char*) * self->loop_stack_capacity);
  }

  self->loop_stack[self->loop_stack_count++] = label_name->value;

  APPEND_DYNAMIC_STR(label_name, "L");
  APPEND_DYNAMIC_STR(label_name, to_string(self->loop_count - 1));

  APPEND_DYNAMIC_STR(self->code, label_name->value);
  APPEND_DYNAMIC_STR(self->code, ":\n");

  for (uint index = 0; index < node->body->count; ++index) {
    compile_stmt(self, node->body->array[index]);
  }
  self->loop_stack_count--;
  APPEND_DYNAMIC_STR(self->code, "jmp ");
  APPEND_DYNAMIC_STR(self->code, label_name->value);
  APPEND_DYNAMIC_STR(self->code, "\n");
  APPEND_DYNAMIC_STR(self->code, label_name->value);
  APPEND_DYNAMIC_STR(self->code, "_END:\n");
  free(label_name);
};

void compile_literal(COMPILER* self, AST_NODE* node) {
  switch (node->kind) {
    case NUMBER_LITERAL: {
      APPEND_DYNAMIC_STR(self->code, "mov ");
      const char* reg = (self->is_rax_in_use == 0) ? "rax, " : "r8, ";
      APPEND_DYNAMIC_STR(self->code, reg);
      self->is_rax_in_use = 1;
      APPEND_DYNAMIC_STR(self->code,
                         to_string(node->VALUE_NUMBER_LITERAL->value));
      APPEND_DYNAMIC_STR(self->code, "\n");
      break;
    }
    case CHAR_LITERAL: {
      APPEND_DYNAMIC_STR(self->code, "mov ");
      const char* reg = (self->is_rax_in_use == 0) ? "rax, " : "r8, ";
      APPEND_DYNAMIC_STR(self->code, reg);
      self->is_rax_in_use = 1;
      APPEND_DYNAMIC_STR(self->code,
                         to_string((int)node->VALUE_CHAR_LITERAL->value));
      APPEND_DYNAMIC_STR(self->code, "\n");
      break;
    }
    case IDENTIFIER_LITERAL: {
      VARIABLE* identifier =
          find_variable(self->stack_frames[self->stack_frame_count - 1],
                        node->VALUE_IDENTIFIER_LITERAL->value);
      if (identifier->str_size[0] == 'B') {
        APPEND_DYNAMIC_STR(self->code, "movzx ");
      } else {
        APPEND_DYNAMIC_STR(self->code, "mov ");
      }
      const char* reg = (self->is_rax_in_use == 0) ? "rax, " : "r8, ";
      APPEND_DYNAMIC_STR(self->code, reg);
      self->is_rax_in_use = 1;
      APPEND_DYNAMIC_STR(self->code, identifier->str_size);
      APPEND_DYNAMIC_STR(self->code, " [rbp");
      const char* sign = identifier->add == 1 ? "+" : "-";
      APPEND_DYNAMIC_STR(self->code, sign)
      APPEND_DYNAMIC_STR(self->code, to_string(identifier->rbp_end));
      APPEND_DYNAMIC_STR(self->code, "]\n");
      break;
    }
    case STRING_LITERAL: {
      string_make(self, node);
      APPEND_DYNAMIC_STR(self->code, "mov ");
      const char* reg = (self->is_rax_in_use == 0) ? "rax, " : "r8, ";
      APPEND_DYNAMIC_STR(self->code, reg);
      self->is_rax_in_use = 1;
      APPEND_DYNAMIC_STR(self->code, "LC");
      APPEND_DYNAMIC_STR(self->code,
                         to_string(find_const_string(
                             self, node->VALUE_STRING_LITERAL->value)));
      APPEND_DYNAMIC_STR(self->code, "\n");
      break;
    }
    default:
      break;
  }
}

void compile_return_expr(COMPILER* self, AST_NODE_RETURN_EXPR* node) {
  compile_stmt(self, node->return_value);
  APPEND_DYNAMIC_STR(self->code, "add rsp, ");
  APPEND_DYNAMIC_STR(
      self->code,
      to_string(8 + self->stack_frames[self->stack_frame_count - 1]->rbp));
  APPEND_DYNAMIC_STR(self->code, "\npop rbp\nret\n")
  self->is_rax_in_use = 0;
}

void compile_call_expr(COMPILER* self, AST_NODE_CALL_EXPR* node) {
  if (node->caller->kind == IDENTIFIER_LITERAL) {
    if (is_global_function(
            self, node->caller->VALUE_IDENTIFIER_LITERAL->value) == 1) {
      for (int index = node->args->count - 1; index >= 0; --index) {
        compile_stmt(self, node->args->array[index]);
        APPEND_DYNAMIC_STR(self->code, "push rax\n");
        self->is_rax_in_use = 0;
      }
      APPEND_DYNAMIC_STR(self->code, "call ");
      APPEND_DYNAMIC_STR(self->code,
                         node->caller->VALUE_IDENTIFIER_LITERAL->value);
      APPEND_DYNAMIC_STR(self->code, "\n");
      APPEND_DYNAMIC_STR(self->code, "add rsp, ");
      APPEND_DYNAMIC_STR(self->code, to_string(8 * node->args->count))
      APPEND_DYNAMIC_STR(self->code, "\n");
    } else {
      for (int index = node->args->count - 1; index >= 0; --index) {
        compile_stmt(self, node->args->array[index]);
        APPEND_DYNAMIC_STR(self->code, "push rax\n");
        self->is_rax_in_use = 0;
      }
      compile_stmt(self, node->caller);
      APPEND_DYNAMIC_STR(self->code, "call rax\n");
      self->is_rax_in_use = 0;
      APPEND_DYNAMIC_STR(self->code, "add rsp, ");
      APPEND_DYNAMIC_STR(self->code, to_string(8 * node->args->count))
      APPEND_DYNAMIC_STR(self->code, "\n");
    }
  } else {
    for (int index = node->args->count - 1; index >= 0; --index) {
      compile_stmt(self, node->args->array[index]);
      APPEND_DYNAMIC_STR(self->code, "push rax\n");
      self->is_rax_in_use = 0;
    }
    compile_stmt(self, node->caller);
    APPEND_DYNAMIC_STR(self->code, "call rax\n");
    self->is_rax_in_use = 0;
    APPEND_DYNAMIC_STR(self->code, "add rsp, ");
    APPEND_DYNAMIC_STR(self->code, to_string(8 * node->args->count))
    APPEND_DYNAMIC_STR(self->code, "\n");
  }
}

void compile_binary_expr(COMPILER* self, AST_NODE_BINARY_EXPR* node) {
  self->is_rax_in_use = 0;
  compile_stmt(self, node->left);
  APPEND_DYNAMIC_STR(self->code, "push rax\n");
  compile_stmt(self, node->right);
  APPEND_DYNAMIC_STR(self->code, "push r8\n");

  APPEND_DYNAMIC_STR(self->code, "pop r8\n");
  APPEND_DYNAMIC_STR(self->code, "pop rax\n");

  if (strcmp(node->operator, "+") == 0)
    APPEND_DYNAMIC_STR(self->code, "add rax, r8\n");
  if (strcmp(node->operator, "-") == 0)
    APPEND_DYNAMIC_STR(self->code, "sub rax, r8\n");
  if (strcmp(node->operator, "*") == 0)
    APPEND_DYNAMIC_STR(self->code, "imul rax, r8\n");
  if (strcmp(node->operator, "/") == 0)
    APPEND_DYNAMIC_STR(self->code, "idiv rax, r8\n");
}

void compile_assignment_expr(COMPILER* self, AST_NODE_BINARY_EXPR* node) {
  if (node->left->kind == IDENTIFIER_LITERAL) {
    VARIABLE* variable =
        find_variable(self->stack_frames[self->stack_frame_count - 1],
                      node->left->VALUE_IDENTIFIER_LITERAL->value);
    compile_stmt(self, node->right);
    APPEND_DYNAMIC_STR(self->code, "mov ");
    APPEND_DYNAMIC_STR(self->code, variable->str_size);
    APPEND_DYNAMIC_STR(self->code, " [rbp-");
    APPEND_DYNAMIC_STR(self->code, to_string(variable->rbp_end));
    APPEND_DYNAMIC_STR(self->code, "], rax\n");
    self->is_rax_in_use = 0;
  }
}

void compile_var_decl(COMPILER* self, AST_NODE_VAR_DECL* node) {
  STACK_FRAME* stack_frame = self->stack_frames[self->stack_frame_count - 1];
  VARIABLE* variable = malloc(sizeof(VARIABLE));
  variable->name = node->name;
  variable->str_size = get_type_offset_name(node->type);
  variable->add = 0;

  if (stack_frame->variable_count >= stack_frame->variable_capacity) {
    stack_frame->variable_capacity *= 2;
    stack_frame->variables =
        realloc(stack_frame->variables,
                sizeof(VARIABLE*) * stack_frame->variable_capacity);
  }

  stack_frame->variables[stack_frame->variable_count++] = variable;

  if (node->type->type_kind == BASE_TYPE) {
    if (strcmp(node->type->base_type, "int") == 0) {
      variable->rbp_start = stack_frame->rbp;
      stack_frame->rbp += get_type_offset(node->type);
      variable->rbp_end = stack_frame->rbp;
      if (node->value->kind == NUMBER_LITERAL) {
        APPEND_DYNAMIC_STR(self->code, "mov QWORD [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(stack_frame->rbp));
        APPEND_DYNAMIC_STR(self->code, "], ");
        APPEND_DYNAMIC_STR(self->code,
                           to_string(node->value->VALUE_NUMBER_LITERAL->value));
      } else {
        compile_stmt(self, node->value);
        APPEND_DYNAMIC_STR(self->code, "mov QWORD [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(stack_frame->rbp));
        APPEND_DYNAMIC_STR(self->code, "], rax");
        self->is_rax_in_use = 0;
      }
      APPEND_DYNAMIC_STR(self->code, "\n");
    } else if (strcmp(node->type->base_type, "char") == 0) {
      variable->rbp_start = stack_frame->rbp;
      stack_frame->rbp += get_type_offset(node->type);
      variable->rbp_end = stack_frame->rbp;
      if (node->value->kind == CHAR_LITERAL) {
        APPEND_DYNAMIC_STR(self->code, "mov BYTE [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(stack_frame->rbp));
        APPEND_DYNAMIC_STR(self->code, "], ");
        APPEND_DYNAMIC_STR(
            self->code, to_string((int)node->value->VALUE_CHAR_LITERAL->value));
        APPEND_DYNAMIC_STR(self->code, "\n");
      } else {
        compile_stmt(self, node->value);
        APPEND_DYNAMIC_STR(self->code, "mov BYTE [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(stack_frame->rbp));
        APPEND_DYNAMIC_STR(self->code, "], rax");
        self->is_rax_in_use = 0;
      }
    }
  } else if (node->type->type_kind == ARRAY_TYPE) {
    variable->rbp_start = stack_frame->rbp;
    stack_frame->rbp += get_type_offset(node->type->array_of) *
                        node->value->VALUE_ARRAY_LITERAL->items->count;
    variable->rbp_end = stack_frame->rbp;

    uint rbp_offset = stack_frame->rbp;

    for (uint index = 0; index < node->value->VALUE_ARRAY_LITERAL->items->count;
         ++index) {
      AST_NODE* item = node->value->VALUE_ARRAY_LITERAL->items->array[index];
      if (item->kind == NUMBER_LITERAL) {
        APPEND_DYNAMIC_STR(self->code, "mov QWORD [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(rbp_offset));
        APPEND_DYNAMIC_STR(self->code, "], ");
        APPEND_DYNAMIC_STR(self->code,
                           to_string(item->VALUE_NUMBER_LITERAL->value));
        APPEND_DYNAMIC_STR(self->code, "\n");
        rbp_offset -= SIZEOF_INT;
      } else if (item->kind == CHAR_LITERAL) {
        APPEND_DYNAMIC_STR(self->code, "mov BYTE [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(rbp_offset));
        APPEND_DYNAMIC_STR(self->code, "], ");
        APPEND_DYNAMIC_STR(self->code,
                           to_string((int)item->VALUE_CHAR_LITERAL->value));
        APPEND_DYNAMIC_STR(self->code, "\n");
        rbp_offset -= SIZEOF_CHAR;
      } else {
        compile_stmt(self, item);
        const char* offset_name = get_type_offset_name(node->type->array_of);
        APPEND_DYNAMIC_STR(self->code, "mov ");
        APPEND_DYNAMIC_STR(self->code, offset_name);
        APPEND_DYNAMIC_STR(self->code, " [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(rbp_offset));
        APPEND_DYNAMIC_STR(self->code, "], rax");
        APPEND_DYNAMIC_STR(self->code, "\n");
        self->is_rax_in_use = 0;
        rbp_offset -= get_type_offset(node->type->array_of);
      }
    }

  } else if (node->type->type_kind == POINTER_TYPE) {
    if (node->type->ptr_to->type_kind == BASE_TYPE) {
      if (strcmp(node->type->ptr_to->base_type, "char") == 0) {
        if (node->value->kind == STRING_LITERAL) {
          string_make(self, node->value);

          APPEND_DYNAMIC_STR(self->code, "lea rax, LC");
          APPEND_DYNAMIC_STR(
              self->code, to_string(find_const_string(
                              self, node->value->VALUE_STRING_LITERAL->value)));
          APPEND_DYNAMIC_STR(self->code, "\n");
        } else {
          compile_stmt(self, node->value);
        }

        variable->rbp_start = stack_frame->rbp;
        stack_frame->rbp += get_type_offset(node->type);
        variable->rbp_end = stack_frame->rbp;
        APPEND_DYNAMIC_STR(self->code, "mov QWORD [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(stack_frame->rbp));
        APPEND_DYNAMIC_STR(self->code, "], rax\n");
        self->is_rax_in_use = 0;
      }
    } else {
      variable->rbp_start = stack_frame->rbp;
      stack_frame->rbp += get_type_offset(node->type);
      variable->rbp_end = stack_frame->rbp;
      AST_NODE_TYPE_DECL* type = node->type;
      while (type->type_kind == POINTER_TYPE) {
        type = type->ptr_to;
      }

      if (type->type_kind == BASE_TYPE && strcmp(type->base_type, "int") == 0) {
        APPEND_DYNAMIC_STR(self->code, "mov QWORD [rbp-");
        APPEND_DYNAMIC_STR(self->code, to_string(stack_frame->rbp));
        APPEND_DYNAMIC_STR(self->code, "], ");
        APPEND_DYNAMIC_STR(self->code,
                           to_string(node->value->VALUE_NUMBER_LITERAL->value));
        APPEND_DYNAMIC_STR(self->code, "\n");
      }
    }
  }
}

void compile_extern_expr(COMPILER* self, AST_NODE_EXTERN_EXPR* node) {
  if (node->is_function == 1) {
    if (self->defined_functions_count >= self->defined_functions_capacity) {
      self->defined_functions_capacity *= 2;
      self->defined_functions =
          realloc(self->defined_functions,
                  sizeof(char*) * self->defined_functions_capacity);
    }

    self->defined_functions[self->defined_functions_count++] =
        (char*)node->symbol;
  }

  APPEND_DYNAMIC_STR(self->code, "extern ")
  APPEND_DYNAMIC_STR(self->code, node->symbol);
  APPEND_DYNAMIC_STR(self->code, "\n");
}

void compile_stmt(COMPILER* self, AST_NODE* node) {
  switch (node->kind) {
    case VAR_DECL_STMT:
      compile_var_decl(self, node->VAR_DECL);
      break;
    case FN_DECL_STMT:
      compile_fn_decl(self, node->FN_DECL);
      break;
    case BINARY_EXPR:
      compile_binary_expr(self, node->BINARY_EXPR);
      break;
    case NUMBER_LITERAL:
    case STRING_LITERAL:
    case CHAR_LITERAL:
    case ARRAY_LITERAL:
    case IDENTIFIER_LITERAL:
      compile_literal(self, node);
      break;
    case ASSIGNMENT_EXPR:
      compile_assignment_expr(self, node->BINARY_EXPR);
      break;
    case CALL_EXPR:
      compile_call_expr(self, node->CALL_EXPR);
      break;
    case RETURN_EXPR:
      compile_return_expr(self, node->RETURN_EXPR);
      break;
    case EXTERN_EXPR:
      compile_extern_expr(self, node->EXTERN_EXPR);
      break;
    case IS_STMT:
      compile_is_stmt(self, node->IS_DECL_STMT);
      break;
    case LOOP_STMT:
      compile_loop_stmt(self, node->LOOP_DECL_STMT);
      break;
    case BREAK_EXPR:
      compile_break_stmt(self, node);
      break;
    case CONTINUE_EXPR:
      compile_continue_stmt(self, node);
      break;
    default:
      break;
  }
}

const char* compile(AST_NODE_ARRAY* ast) {
  COMPILER* compiler = malloc(sizeof(COMPILER));
  compiler->stack_frame_count = 0;
  compiler->stack_frame_capacity = 16;
  compiler->stack_frames =
      malloc(sizeof(STACK_FRAME*) * compiler->stack_frame_capacity);
  compiler->ast = ast;

  DYNAMIC_STR* code = malloc(sizeof(DYNAMIC_STR));
  code->count = 0;
  code->capacity = 16;
  code->value = malloc(sizeof(char) * code->capacity);
  code->value[0] = '\0';

  DYNAMIC_STR* top_code = malloc(sizeof(DYNAMIC_STR));
  top_code->count = 0;
  top_code->capacity = 16;
  top_code->value = malloc(sizeof(char) * top_code->capacity);
  top_code->value[0] = '\0';

  compiler->strings_count = 0;
  compiler->strings_capacity = 16;
  compiler->strings =
      malloc(sizeof(STRING_MEMORY_ADDRESS*) * compiler->strings_capacity);

  compiler->code = code;
  compiler->top_code = top_code;
  compiler->is_rax_in_use = 0;
  compiler->loop_count = 0;
  compiler->is_count = 0;

  compiler->loop_stack_count = 0;
  compiler->loop_stack_capacity = 16;
  compiler->loop_stack = malloc(sizeof(char*) * compiler->loop_stack_count);

  compiler->defined_functions_count = 0;
  compiler->defined_functions_capacity = 16;
  compiler->defined_functions =
      malloc(sizeof(char*) * compiler->defined_functions_capacity);

  APPEND_DYNAMIC_STR(top_code, "global main\nsection .data\n");
  APPEND_DYNAMIC_STR(code, "section .text\n")
  for (uint index = 0; index < ast->count; ++index) {
    compile_stmt(compiler, ast->array[index]);
  }

  APPEND_DYNAMIC_STR(top_code, code->value);
  return top_code->value;
}