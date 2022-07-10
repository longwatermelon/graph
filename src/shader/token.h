#ifndef SHADER_TOKEN_H
#define SHADER_TOKEN_H

enum
{
    TT_ID,
    TT_INT,
    TT_SEMI,
    TT_LPAREN,
    TT_RPAREN,
    TT_LBRACE,
    TT_RBRACE,
    TT_EQUAL,
    TT_COMMA,
    TT_EOF
};

struct Token
{
    int type;
    char *value;
};

// Value must be heap allocated, will be freed in token_free
struct Token *token_alloc(int type, char *value);
void token_free(struct Token *t);

#endif
