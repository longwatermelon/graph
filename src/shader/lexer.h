#ifndef SHADER_LEXER_H
#define SHADER_LEXER_H

#include "token.h"
#include <sys/types.h>

struct Lexer
{
    char ch;
    size_t index;

    char *contents;
    size_t line_num;
};

// Contents must be heap allocated, lexer will free in lexer_free
struct Lexer *lexer_alloc(char *contents);
void lexer_free(struct Lexer *l);

void lexer_advance(struct Lexer *l);

char *lexer_collect_int(struct Lexer *l);
char *lexer_collect_id(struct Lexer *l);

struct Token *lexer_next_token(struct Lexer *l);
struct Token *lexer_advance_with_token(struct Lexer *l, struct Token *t);

#endif

