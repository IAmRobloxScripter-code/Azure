#pragma once
#include <stdlib.h>
#include <string.h>

typedef enum LEX_TOKEN_KIND {
  TOK_TYPE,
  TOK_PLUS,
  TOK_MINUS,
  TOK_ASTERIK,
  TOK_SLASH,
  TOK_EQUAL,
  TOK_OPEN_PAREN,
  TOK_CLOSING_PAREN,
  TOK_OPEN_SQUARE_PAREN,
  TOK_CLOSING_SQUARE_PAREN,
  TOK_EXTERN,
  TOK_LET,
  TOK_FN,
  TOK_IS,
  TOK_ELSE,
  TOK_RETURN,
  TOK_BREAK,
  TOK_CONTINUE,
  TOK_LOOP,
  TOK_NUMBER_VALUE,
  TOK_STRING_VALUE,
  TOK_CHAR_VALUE,
  TOK_IDENTIFIER,
  TOK_EOF
} LEX_TOKEN_KIND;

typedef unsigned int uint;

typedef struct LEX_TOKEN {
  LEX_TOKEN_KIND kind;
  uint line;
  const char* value;
} LEX_TOKEN;

extern const char* ALPHA;

typedef struct LEX_TOKEN_ARRAY {
  uint count;
  uint capacity;
  LEX_TOKEN** array;
} LEX_TOKEN_ARRAY;

#define GROW_LEX_TOKEN_ARRAY(vec, insert)                                 \
  if (vec->count >= vec->capacity) {                                      \
    vec->capacity *= 2;                                                   \
    vec->array = realloc(vec->array, sizeof(LEX_TOKEN*) * vec->capacity); \
  }                                                                       \
  vec->array[vec->count] = insert;                                        \
  vec->count++;

int is_in_str(const char* str, const char c);

typedef struct DYNAMIC_STR {
  uint count;
  uint capacity;
  char* value;
} DYNAMIC_STR;

#define GROW_DYNAMIC_STR(str, character)             \
  if (str->count + 1 >= str->capacity) {             \
    str->capacity *= 2;                              \
    str->value = realloc(str->value, str->capacity); \
  }                                                  \
  str->value[str->count++] = character;              \
  str->value[str->count] = '\0';

#define APPEND_DYNAMIC_STR(str, str2)                                        \
  for (uint __apnd_index = 0; __apnd_index < strlen(str2); ++__apnd_index) { \
    GROW_DYNAMIC_STR(str, str2[__apnd_index])                                \
  }

typedef enum AST_KIND {
  NUMBER_LITERAL,
  CHAR_LITERAL,
  STRING_LITERAL,
  ARRAY_LITERAL,
  IDENTIFIER_LITERAL,

  VAR_DECL_STMT,
  FN_DECL_STMT,
  LOOP_STMT,
  IS_STMT,
  RETURN_EXPR,
  BREAK_EXPR,
  CONTINUE_EXPR,
  CALL_EXPR,

  BINARY_EXPR,
  ASSIGNMENT_EXPR,
  UNARY_EXPR,
  EXTERN_EXPR
} AST_KIND;

typedef struct AST_NODE_VAR_DECL AST_NODE_VAR_DECL;
typedef struct AST_NODE_FN_DECL AST_NODE_FN_DECL;
typedef struct AST_NODE_NUMBER_LITERAL AST_NODE_NUMBER_LITERAL;
typedef struct AST_NODE_STRING_LITERAL AST_NODE_STRING_LITERAL;
typedef struct AST_NODE_CHAR_LITERAL AST_NODE_CHAR_LITERAL;
typedef struct AST_NODE_IDENTIFIER_LITERAL AST_NODE_IDENTIFIER_LITERAL;
typedef struct AST_NODE_ARRAY_LITERAL AST_NODE_ARRAY_LITERAL;
typedef struct AST_NODE_BINARY_EXPR AST_NODE_BINARY_EXPR;
typedef struct AST_NODE_CALL_EXPR AST_NODE_CALL_EXPR;
typedef struct AST_NODE_RETURN_EXPR AST_NODE_RETURN_EXPR;
typedef struct AST_NODE_EXTERN_EXPR AST_NODE_EXTERN_EXPR;
typedef struct AST_NODE_TYPE_DECL AST_NODE_TYPE_DECL;
typedef struct AST_NODE_ARRAY AST_NODE_ARRAY;

typedef struct AST_NODE {
  AST_KIND kind;
  union {
    AST_NODE_NUMBER_LITERAL* VALUE_NUMBER_LITERAL;
    AST_NODE_STRING_LITERAL* VALUE_STRING_LITERAL;
    AST_NODE_CHAR_LITERAL* VALUE_CHAR_LITERAL;
    AST_NODE_IDENTIFIER_LITERAL* VALUE_IDENTIFIER_LITERAL;
    AST_NODE_ARRAY_LITERAL* VALUE_ARRAY_LITERAL;

    AST_NODE_VAR_DECL* VAR_DECL;
    AST_NODE_FN_DECL* FN_DECL;
    AST_NODE_BINARY_EXPR* BINARY_EXPR;
    AST_NODE_CALL_EXPR* CALL_EXPR;
    AST_NODE_RETURN_EXPR* RETURN_EXPR;
    AST_NODE_EXTERN_EXPR* EXTERN_EXPR;
  };
} AST_NODE;

struct AST_NODE_EXTERN_EXPR {
  const char* symbol;
  int is_function;
};

struct AST_NODE_RETURN_EXPR {
  AST_NODE* return_value;
};

struct AST_NODE_CALL_EXPR {
  AST_NODE* caller;
  AST_NODE_ARRAY* args;
};

typedef enum AST_TYPE_DECL_KINDS {
  BASE_TYPE,
  POINTER_TYPE,
  ARRAY_TYPE,
} AST_TYPE_DECL_KINDS;

struct AST_NODE_TYPE_DECL {
  AST_TYPE_DECL_KINDS type_kind;
  union {
    AST_NODE_TYPE_DECL* ptr_to;
    const char* base_type;
    AST_NODE_TYPE_DECL* array_of;
  };
};

struct AST_NODE_VAR_DECL {
  AST_NODE_TYPE_DECL* type;
  AST_NODE* value;
  const char* name;
};

typedef struct AST_NODE_PARAM {
  AST_NODE_TYPE_DECL* type;
  const char* name;
} AST_NODE_PARAM;

struct AST_NODE_FN_DECL {
  AST_NODE_TYPE_DECL* return_type;
  AST_NODE_PARAM** params;
  AST_NODE_ARRAY* body;
  uint param_count;
  const char* name;
};

struct AST_NODE_BINARY_EXPR {
  char operator;
  AST_NODE* left;
  AST_NODE* right;
};

struct AST_NODE_NUMBER_LITERAL {
  uint value;
};

struct AST_NODE_STRING_LITERAL {
  const char* value;
};

struct AST_NODE_CHAR_LITERAL {
  char value;
};

struct AST_NODE_IDENTIFIER_LITERAL {
  const char* value;
};

struct AST_NODE_ARRAY_LITERAL {
  AST_NODE_ARRAY* items;
};

struct AST_NODE_ARRAY {
  AST_NODE** array;
  uint count;
  uint capacity;
};

typedef struct PARSER {
  LEX_TOKEN_ARRAY* tokens;
  AST_NODE_ARRAY* ast;
  uint current_token;
  const char* file_name;
} PARSER;

#define GROW_AST_NODE_ARRAY(vec, insert)                                 \
  if (vec->count >= vec->capacity) {                                     \
    vec->capacity *= 2;                                                  \
    vec->array = realloc(vec->array, sizeof(AST_NODE*) * vec->capacity); \
  }                                                                      \
  vec->array[vec->count] = insert;                                       \
  vec->count++;

AST_NODE_ARRAY* parse_tokens(LEX_TOKEN_ARRAY* tokens, const char* file_name);
LEX_TOKEN_ARRAY* lex_input_str(const char* input);