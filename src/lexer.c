#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

enum TokenType {
  KEYWORD_LABEL_VALUE,
  OP_EQUALS,
  STATEMENT_TERMINATOR,
};

typedef struct _Token {
  int token_type;
  char *lexemme;
  char *literal;
  unsigned long line;
} Token;

typedef struct _Tokens {
  size_t array_size;
  size_t token_count;
  Token *tokens;
} Tokens;

unsigned int initialize_tokens_container(Tokens **_tokens) {
  size_t array_size = 64;
  Tokens *tokens = malloc(sizeof(Tokens));
  tokens->tokens = malloc(sizeof(Token) * array_size);
  tokens->token_count = 0;
  tokens->array_size = array_size;
  *_tokens = tokens;
  return 0;
}

unsigned int add_token(Tokens *tokens) {
  if (tokens->token_count >= tokens->array_size) {
    Token *tokens_orig = tokens->tokens;
    if (realloc(tokens->tokens, sizeof(Token) * tokens->array_size * 2)) {
      tokens->tokens = tokens_orig;
      return -1;
    };
  }
  Token new_token = {
    .token_type = 1,
  };
  tokens->tokens[tokens->token_count] = new_token;
  return 0;
}

unsigned int parse(FILE *stream) {
  wchar_t wchar;
  while ((wchar = getwc(stream)) != WEOF) {
    
  }
  return 0;
}
