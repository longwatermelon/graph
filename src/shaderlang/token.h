#ifndef SHADER_TOKEN_H
#define SHADER_TOKEN_H

typedef enum
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
    TT_FLOAT,
    TT_BINOP,
    TT_EOF
} TokenType;

typedef enum
{
    BT_PLUS
} BinopToken;

struct Token
{
    int type;
    char *value;

    BinopToken binop;
};

// Value must be heap allocated, will be freed in token_free
struct Token *token_alloc(TokenType type, char *value);
struct Token *token_binop_alloc(BinopToken type, char *value);
void token_free(struct Token *t);

#endif

