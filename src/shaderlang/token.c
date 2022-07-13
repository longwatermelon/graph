#include "token.h"
#include <stdlib.h>


struct Token *token_alloc(TokenType type, char *value)
{
    struct Token *t = malloc(sizeof(struct Token));
    t->type = type;
    t->value = value;

    t->binop = 0;

    return t;
}


struct Token *token_binop_alloc(BinopToken type, char *value)
{
    struct Token *t = token_alloc(TT_BINOP, value);
    t->binop = type;

    return t;
}


void token_free(struct Token *t)
{
    free(t->value);
    free(t);
}

