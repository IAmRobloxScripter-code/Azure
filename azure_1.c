#include "azure_1.h"

#include <stdio.h>

const char* ALPHA = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";

int is_in_str(const char* str, const char character) {
  uint str_length = strlen(str);
  for (uint index = 0; index < str_length; ++index) {
    if (str[index] == character) {
      return 1;
    }
  }
  return 0;
};

const char* lex_token_kind_to_string(LEX_TOKEN_KIND kind) {
  switch (kind) {
    case TOK_TYPE:
      return "TOK_TYPE";
    case TOK_PLUS:
      return "TOK_PLUS";
    case TOK_MINUS:
      return "TOK_MINUS";
    case TOK_ASTERIK:
      return "TOK_ASTERIK";
    case TOK_SLASH:
      return "TOK_SLASH";
    case TOK_EQUAL:
      return "TOK_EQUAL";
    case TOK_OPEN_PAREN:
      return "TOK_OPEN_PAREN";
    case TOK_CLOSING_PAREN:
      return "TOK_CLOSING_PAREN";
    case TOK_OPEN_SQUARE_PAREN:
      return "TOK_OPEN_SQUARE_PAREN";
    case TOK_CLOSING_SQUARE_PAREN:
      return "TOK_CLOSING_SQUARE_PAREN";
    case TOK_LET:
      return "TOK_LET";
    case TOK_FN:
      return "TOK_FN";
    case TOK_IS:
      return "TOK_IS";
    case TOK_ELSE:
      return "TOK_ELSE";
    case TOK_RETURN:
      return "TOK_RETURN";
    case TOK_BREAK:
      return "TOK_BREAK";
    case TOK_CONTINUE:
      return "TOK_CONTINUE";
    case TOK_LOOP:
      return "TOK_LOOP";
    case TOK_NUMBER_VALUE:
      return "TOK_NUMBER_VALUE";
    case TOK_STRING_VALUE:
      return "TOK_STRING_VALUE";
    case TOK_CHAR_VALUE:
      return "TOK_CHAR_VALUE";
    case TOK_IDENTIFIER:
      return "TOK_IDENTIFIER";
    case TOK_EXTERN:
      return "TOK_EXTERN";
    case TOK_EOF:
      return "TOK_EOF";
    default:
      return "TOK_UNKNOWN";
  }
}

LEX_TOKEN_KIND get_keyword_kind(const char* value) {
  if (strcmp(value, "loop") == 0)
    return TOK_LOOP;
  else if (strcmp(value, "is") == 0)
    return TOK_IS;
  else if (strcmp(value, "break") == 0)
    return TOK_BREAK;
  else if (strcmp(value, "continue") == 0)
    return TOK_CONTINUE;
  else if (strcmp(value, "ret") == 0)
    return TOK_RETURN;
  else if (strcmp(value, "else") == 0)
    return TOK_ELSE;
  else if (strcmp(value, "int") == 0)
    return TOK_TYPE;
  else if (strcmp(value, "char") == 0)
    return TOK_TYPE;
  else if (strcmp(value, "let") == 0)
    return TOK_LET;
  else if (strcmp(value, "fn") == 0)
    return TOK_FN;
  else if (strcmp(value, "extern") == 0)
    return TOK_EXTERN;
  else
    return TOK_IDENTIFIER;
}

LEX_TOKEN* make_token(LEX_TOKEN_KIND kind, const char* value, uint line) {
  LEX_TOKEN* token = malloc(sizeof(LEX_TOKEN));
  token->kind = kind;
  token->line = line;
  token->value = value;

  return token;
}

LEX_TOKEN_ARRAY* lex_input_str(const char* input) {
  LEX_TOKEN_ARRAY* tokens = malloc(sizeof(LEX_TOKEN_ARRAY));
  tokens->count = 0;
  tokens->capacity = 16;
  tokens->array = malloc(sizeof(LEX_TOKEN*) * tokens->capacity);

  uint index = 0;
  uint line = 1;

  while (input[index] != '\0') {
    char character = input[index];

    if (character == '\n' || character == '\r') {
      index++;
      line += 1;
      continue;
    }

    if (character == ' ' || character == '\t') {
      index++;
      continue;
    }

    if (character == '(') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_OPEN_PAREN, "(", line));
      index++;
    } else if (character == ')') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_CLOSING_PAREN, ")", line));
      index++;
    } else if (character == '[') {
      GROW_LEX_TOKEN_ARRAY(tokens,
                           make_token(TOK_OPEN_SQUARE_PAREN, "[", line));
      index++;
    } else if (character == ']') {
      GROW_LEX_TOKEN_ARRAY(tokens,
                           make_token(TOK_CLOSING_SQUARE_PAREN, "]", line));
      index++;
    } else if (character == '+') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_PLUS, "+", line));
      index++;
    } else if (character == '-') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_MINUS, "-", line));
      index++;
    } else if (character == '*') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_ASTERIK, "*", line));
      index++;
    } else if (character == '/') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_SLASH, "/", line));
      index++;
    } else if (character == '=') {
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_EQUAL, "=", line));
      index++;
    } else if (character >= '0' && character <= '9') {
      DYNAMIC_STR* number = malloc(sizeof(DYNAMIC_STR));
      number->count = 0;
      number->capacity = 16;
      number->value = malloc(sizeof(char) * number->capacity);
      number->value[0] = '\0';

      while (input[index] != '\0' &&
             (input[index] >= '0' && input[index] <= '9')) {
        GROW_DYNAMIC_STR(number, input[index])
        index++;
      }

      GROW_LEX_TOKEN_ARRAY(tokens,
                           make_token(TOK_NUMBER_VALUE, number->value, line));
      free(number);
    } else if (is_in_str(ALPHA, character)) {
      DYNAMIC_STR* identifier = malloc(sizeof(DYNAMIC_STR));
      identifier->count = 0;
      identifier->capacity = 16;
      identifier->value = malloc(sizeof(char) * identifier->capacity);
      identifier->value[0] = '\0';

      while (input[index] != '\0' &&
             (is_in_str(ALPHA, input[index]) ||
              (input[index] >= '0' && input[index] <= '9'))) {
        GROW_DYNAMIC_STR(identifier, input[index])
        index++;
      }

      GROW_LEX_TOKEN_ARRAY(tokens,
                           make_token(get_keyword_kind(identifier->value),
                                      identifier->value, line));
      free(identifier);
    } else if (character == '"') {
      DYNAMIC_STR* str = malloc(sizeof(DYNAMIC_STR));
      str->count = 0;
      str->capacity = 16;
      str->value = malloc(sizeof(char) * str->capacity);
      str->value[0] = '\0';
      index++;

      while (input[index] != '\0' && input[index] != '"') {
        if (input[index] == '\\') {
          if (input[index + 1] == '\\') {
            GROW_DYNAMIC_STR(str, '\\');
            index += 2;
            continue;
          } else if (input[index + 1] == 'n') {
            GROW_DYNAMIC_STR(str, '\n');
            index += 2;
            continue;
          } else if (input[index + 1] == 't') {
            GROW_DYNAMIC_STR(str, '\t');
            index += 2;
            continue;
          } else if (input[index + 1] == 'r') {
            GROW_DYNAMIC_STR(str, '\r');
            index += 2;
            continue;
          } else if (input[index + 1] == '0') {
            GROW_DYNAMIC_STR(str, '\0');
            index += 2;
            continue;
          } else if (input[index + 1] == '"') {
            GROW_DYNAMIC_STR(str, '"');
            index += 2;
            continue;
          } else if (input[index + 1] == '\'') {
            GROW_DYNAMIC_STR(str, '\'');
            index += 2;
            continue;
          }
          index++;
          GROW_DYNAMIC_STR(str, input[index])
        } else {
          GROW_DYNAMIC_STR(str, input[index]);
        }
        index++;
      }

      index++;
      GROW_LEX_TOKEN_ARRAY(tokens,
                           make_token(TOK_STRING_VALUE, str->value, line));
      free(str);
    } else if (character == '\'') {
      index++;
      char* ch = malloc(2);

      if (input[index] == '\\') {
        if (input[index + 1] == '\\') {
          ch[0] = '\\';
          index += 2;
        } else if (input[index + 1] == 'n') {
          ch[0] = '\n';
          index += 2;
        } else if (input[index + 1] == 't') {
          ch[0] = '\t';
          index += 2;
        } else if (input[index + 1] == 'r') {
          ch[0] = '\r';
          index += 2;
        } else if (input[index + 1] == '0') {
          ch[0] = '\0';
          index += 2;
        } else if (input[index + 1] == '"') {
          ch[0] = '"';
          index += 2;
        } else if (input[index + 1] == '\'') {
          ch[0] = '\'';
          index += 2;
        } else {
          ch[0] = input[index];
          index += 2;
        }
      } else {
        ch[0] = input[index];
        index++;
      }
      ch[1] = '\0';

      index++;
      GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_CHAR_VALUE, ch, line));
    }
  }

  GROW_LEX_TOKEN_ARRAY(tokens, make_token(TOK_EOF, "EOF", line));
  return tokens;
}

LEX_TOKEN* at(PARSER* self) { return self->tokens->array[self->current_token]; }

LEX_TOKEN* eat(PARSER* self) {
  return self->tokens->array[self->current_token++];
}

LEX_TOKEN* expect(PARSER* self, LEX_TOKEN_KIND kind) {
  if (at(self)->kind == kind) {
    return eat(self);
  } else {
    fprintf(stderr, "[%s:%u]: Expected %s, got %s instead!\n", self->file_name,
            at(self)->line, lex_token_kind_to_string(kind),
            lex_token_kind_to_string(at(self)->kind));
    exit(1);
    return 0;
  }
}

AST_NODE* make_ast_node(AST_KIND kind) {
  AST_NODE* node = malloc(sizeof(AST_NODE));
  node->kind = kind;
  return node;
}

AST_NODE* parse_stmt(PARSER* self);
AST_NODE* parse_var_decl(PARSER* self);
AST_NODE* parse_fn_decl(PARSER* self);
AST_NODE* parse_expr(PARSER* self);
AST_NODE* parse_binary_expr(PARSER* self);
AST_NODE* parse_square_brackets(PARSER* self);
AST_NODE* parse_primary_expr(PARSER* self);
AST_NODE* parse_extern_expr(PARSER* self);
AST_NODE* parse_return_expr(PARSER* self);
AST_NODE_TYPE_DECL* parse_type(PARSER* self);

AST_NODE* parse_stmt(PARSER* self) {
  free(expect(self, TOK_OPEN_PAREN));
  LEX_TOKEN* current = at(self);
  AST_NODE* node = 0;
  switch (current->kind) {
    case TOK_LET:
      node = parse_var_decl(self);
      break;
    case TOK_FN:
      node = parse_fn_decl(self);
      break;
    case TOK_PLUS:
    case TOK_MINUS:
    case TOK_ASTERIK:
    case TOK_SLASH:
    case TOK_EQUAL:
      node = parse_binary_expr(self);
      break;
    case TOK_OPEN_SQUARE_PAREN:
      node = parse_square_brackets(self);
      break;

    case TOK_RETURN:
      node = parse_return_expr(self);
      break;
    case TOK_EXTERN:
      node = parse_extern_expr(self);
      break;
    default:
      node = parse_expr(self);
      break;
  }
  free(expect(self, TOK_CLOSING_PAREN));
  return node;
}

AST_NODE* parse_square_brackets(PARSER* self) {
  free(eat(self));
  if (at(self)->kind == TOK_CLOSING_SQUARE_PAREN) {
    free(eat(self));
    AST_NODE* node = make_ast_node(ARRAY_LITERAL);
    AST_NODE_ARRAY_LITERAL* array_literal_node =
        malloc(sizeof(AST_NODE_ARRAY_LITERAL));
    AST_NODE_ARRAY* array = malloc(sizeof(AST_NODE_ARRAY));
    array->count = 0;
    array->capacity = 16;
    array->array = malloc(sizeof(AST_NODE*) * array->capacity);
    array_literal_node->items = array;
    node->VALUE_ARRAY_LITERAL = array_literal_node;

    while (at(self)->kind != TOK_EOF && at(self)->kind != TOK_CLOSING_PAREN) {
      if (at(self)->kind == TOK_OPEN_PAREN) {
        GROW_AST_NODE_ARRAY(array, parse_stmt(self));
      } else {
        GROW_AST_NODE_ARRAY(array, parse_expr(self));
      }
    }

    return node;
  }
  return 0;
}

AST_NODE_TYPE_DECL* parse_type(PARSER* self) {
  AST_NODE_TYPE_DECL* node = malloc(sizeof(AST_NODE_TYPE_DECL));

  if (at(self)->kind == TOK_ASTERIK) {
    free(eat(self));
    node->type_kind = POINTER_TYPE;
    node->ptr_to = parse_type(self);
  } else if (at(self)->kind == TOK_TYPE) {
    LEX_TOKEN* type = eat(self);
    node->type_kind = BASE_TYPE;
    node->base_type = type->value;
    free(type);
  } else if (at(self)->kind == TOK_OPEN_SQUARE_PAREN) {
    free(eat(self));
    free(expect(self, TOK_CLOSING_SQUARE_PAREN));
    node->type_kind = ARRAY_TYPE;
    node->array_of = parse_type(self);
  } else if (at(self)->kind == TOK_OPEN_PAREN) {
    free(eat(self));
    free(node);
    node = parse_type(self);
    free(expect(self, TOK_CLOSING_PAREN));
  } else {
    fprintf(stderr, "[%s:%u]: Unexpected type token: %s!\n", self->file_name,
            at(self)->line, lex_token_kind_to_string(at(self)->kind));
    exit(1);
  }

  return node;
}

AST_NODE* parse_primary_expr(PARSER* self) {
  LEX_TOKEN* current = at(self);
  switch (current->kind) {
    case TOK_NUMBER_VALUE: {
      current = eat(self);
      AST_NODE* node = make_ast_node(NUMBER_LITERAL);
      AST_NODE_NUMBER_LITERAL* number_literal_node =
          malloc(sizeof(AST_NODE_NUMBER_LITERAL));
      number_literal_node->value = atoi(current->value);
      node->VALUE_NUMBER_LITERAL = number_literal_node;
      free(current);
      return node;
    }
    case TOK_STRING_VALUE: {
      current = eat(self);
      AST_NODE* node = make_ast_node(STRING_LITERAL);
      AST_NODE_STRING_LITERAL* string_literal_node =
          malloc(sizeof(AST_NODE_STRING_LITERAL));
      string_literal_node->value = current->value;
      node->VALUE_STRING_LITERAL = string_literal_node;
      free(current);
      return node;
    }
    case TOK_CHAR_VALUE: {
      current = eat(self);
      AST_NODE* node = make_ast_node(CHAR_LITERAL);
      AST_NODE_CHAR_LITERAL* char_literal_node =
          malloc(sizeof(AST_NODE_CHAR_LITERAL));
      char_literal_node->value = current->value[0];
      node->VALUE_CHAR_LITERAL = char_literal_node;
      free(current);
      return node;
    }
    case TOK_IDENTIFIER: {
      current = eat(self);
      AST_NODE* node = make_ast_node(IDENTIFIER_LITERAL);
      AST_NODE_IDENTIFIER_LITERAL* identifier_literal_node =
          malloc(sizeof(AST_NODE_IDENTIFIER_LITERAL));
      identifier_literal_node->value = current->value;
      node->VALUE_IDENTIFIER_LITERAL = identifier_literal_node;
      free(current);
      return node;
    }
    default:
      fprintf(stderr, "[%s:%u]: Invalid token %s | %s!\n", self->file_name,
              current->line, current->value,
              lex_token_kind_to_string(current->kind));
      exit(1);
      return 0;
  }
}

AST_NODE* parse_return_expr(PARSER* self) {
  free(eat(self));
  AST_NODE* node = make_ast_node(RETURN_EXPR);
  AST_NODE_RETURN_EXPR* return_expr_node = malloc(sizeof(AST_NODE_RETURN_EXPR));
  return_expr_node->return_value =
      at(self)->kind == TOK_OPEN_PAREN ? parse_stmt(self) : parse_expr(self);
  node->RETURN_EXPR = return_expr_node;
  return node;
}

AST_NODE* parse_call_expr(PARSER* self) {
  if (at(self)->kind == TOK_OPEN_PAREN) {
    free(eat(self));
    free(expect(self, TOK_CLOSING_PAREN));
    AST_NODE* caller =
        at(self)->kind == TOK_OPEN_PAREN ? parse_stmt(self) : parse_expr(self);

    AST_NODE_ARRAY* args = malloc(sizeof(AST_NODE_ARRAY));
    args->count = 0;
    args->capacity = 16;
    args->array = malloc(sizeof(AST_NODE*) * args->capacity);

    AST_NODE* node = make_ast_node(CALL_EXPR);
    AST_NODE_CALL_EXPR* call_expr_node = malloc(sizeof(AST_NODE_CALL_EXPR));
    call_expr_node->caller = caller;
    call_expr_node->args = args;
    node->CALL_EXPR = call_expr_node;

    while (at(self)->kind != TOK_EOF && at(self)->kind != TOK_CLOSING_PAREN) {
      if (at(self)->kind == TOK_OPEN_PAREN) {
        GROW_AST_NODE_ARRAY(args, parse_stmt(self));
      } else {
        GROW_AST_NODE_ARRAY(args, parse_expr(self));
      }
    }

    return node;
  }

  return parse_primary_expr(self);
}

AST_NODE* parse_extern_expr(PARSER* self) {
  free(eat(self));
  LEX_TOKEN* symbol = expect(self, TOK_IDENTIFIER);
  AST_NODE* node = make_ast_node(EXTERN_EXPR);

  AST_NODE_EXTERN_EXPR* extern_expr_node = malloc(sizeof(AST_NODE_EXTERN_EXPR));
  extern_expr_node->symbol = symbol->value;

  if (at(self)->kind == TOK_FN) {
    free(eat(self));
    extern_expr_node->is_function = 1;
  } else {
    extern_expr_node->is_function = 0;
  }

  node->EXTERN_EXPR = extern_expr_node;
  free(symbol);
  return node;
}

AST_NODE* parse_binary_expr(PARSER* self) {
  LEX_TOKEN* operator_token = eat(self);
  AST_NODE* left =
      at(self)->kind == TOK_OPEN_PAREN ? parse_stmt(self) : parse_expr(self);
  AST_NODE* right =
      at(self)->kind == TOK_OPEN_PAREN ? parse_stmt(self) : parse_expr(self);

  AST_NODE* node = make_ast_node(
      operator_token->value[0] != '=' ? BINARY_EXPR : ASSIGNMENT_EXPR);
  AST_NODE_BINARY_EXPR* binary_expr_node = malloc(sizeof(AST_NODE_BINARY_EXPR));
  binary_expr_node->operator = operator_token->value[0];
  binary_expr_node->left = left;
  binary_expr_node->right = right;
  node->BINARY_EXPR = binary_expr_node;
  free(operator_token);
  return node;
}

AST_NODE* parse_expr(PARSER* self) { return parse_call_expr(self); }

AST_NODE* parse_var_decl(PARSER* self) {
  free(eat(self));
  LEX_TOKEN* identifier = expect(self, TOK_IDENTIFIER);
  AST_NODE_TYPE_DECL* type = parse_type(self);
  AST_NODE* value =
      at(self)->kind == TOK_OPEN_PAREN ? parse_stmt(self) : parse_expr(self);

  AST_NODE* node = make_ast_node(VAR_DECL_STMT);
  AST_NODE_VAR_DECL* var_decl_node = malloc(sizeof(AST_NODE_VAR_DECL));
  var_decl_node->type = type;
  var_decl_node->value = value;
  var_decl_node->name = identifier->value;
  node->VAR_DECL = var_decl_node;
  free(identifier);
  return node;
}

AST_NODE* parse_fn_decl(PARSER* self) {
  free(eat(self));
  LEX_TOKEN* identifier = expect(self, TOK_IDENTIFIER);
  AST_NODE_TYPE_DECL* type = parse_type(self);

  free(expect(self, TOK_OPEN_PAREN));

  AST_NODE_PARAM** params = malloc(sizeof(AST_NODE_PARAM*) * 4);
  uint param_count = 0;
  uint param_capacity = 4;

  while (at(self)->kind != TOK_EOF) {
    if (at(self)->kind == TOK_CLOSING_PAREN) break;
    AST_NODE_PARAM* param = malloc(sizeof(AST_NODE_PARAM));
    LEX_TOKEN* param_name = expect(self, TOK_IDENTIFIER);
    param->type = parse_type(self);
    param->name = param_name->value;
    free(param_name);

    if (param_count >= param_capacity) {
      param_capacity *= 2;
      params = realloc(params, sizeof(AST_NODE_PARAM*) * param_capacity);
    }

    params[param_count++] = param;
  }

  free(expect(self, TOK_CLOSING_PAREN));
  AST_NODE_ARRAY* body = malloc(sizeof(AST_NODE_ARRAY));
  body->count = 0;
  body->capacity = 16;
  body->array = malloc(sizeof(AST_NODE*) * body->capacity);

  while (at(self)->kind != TOK_EOF && at(self)->kind != TOK_CLOSING_PAREN) {
    GROW_AST_NODE_ARRAY(body, parse_stmt(self));
  }

  AST_NODE* node = make_ast_node(FN_DECL_STMT);
  AST_NODE_FN_DECL* fn_decl_node = malloc(sizeof(AST_NODE_FN_DECL));
  fn_decl_node->return_type = type;
  fn_decl_node->params = params;
  fn_decl_node->body = body;
  fn_decl_node->param_count = param_count;
  fn_decl_node->name = identifier->value;
  node->FN_DECL = fn_decl_node;
  free(identifier);
  return node;
}

AST_NODE_ARRAY* parse_tokens(LEX_TOKEN_ARRAY* tokens, const char* file_name) {
  PARSER* parser = malloc(sizeof(PARSER));
  parser->tokens = tokens;
  parser->file_name = file_name;

  AST_NODE_ARRAY* ast = malloc(sizeof(AST_NODE_ARRAY));
  ast->count = 0;
  ast->capacity = 16;
  ast->array = malloc(sizeof(AST_NODE*) * ast->capacity);

  parser->ast = ast;
  parser->current_token = 0;

  while (tokens->array[parser->current_token]->kind != TOK_EOF) {
    GROW_AST_NODE_ARRAY(ast, parse_stmt(parser));
  }

  free(tokens);
  return ast;
}