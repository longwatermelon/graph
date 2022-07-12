#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


struct Lexer *lexer_alloc(char *contents)
{
    struct Lexer *l = malloc(sizeof(struct Lexer));
    l->contents = contents;
    l->ch = contents[0];
    l->index = 0;
    l->line_num = 0;

    return l;
}


void lexer_free(struct Lexer *l)
{
    free(l->contents);
    free(l);
}


void lexer_advance(struct Lexer *l)
{
    if (l->ch != '\0' && l->index < strlen(l->contents))
    {
        ++l->index;
        l->ch = l->contents[l->index];
    }
}


char *lexer_collect_int(struct Lexer *l)
{
    size_t start = l->index;

    while (isdigit(l->ch) && l->ch != '\n' && l->ch != '\0')
        lexer_advance(l);

    char *integer = malloc((l->index - start + 1) * sizeof(char));
    integer[0] = '\0';
    strncat(integer, &l->contents[start], sizeof(char) * (l->index - start));

    return integer;
}


char *lexer_collect_id(struct Lexer *l)
{
    size_t start = l->index;

    while ((isalnum(l->ch) || l->ch == '_') && l->ch != '\n' && l->ch != '\0')
        lexer_advance(l);

    char *id = malloc((l->index - start + 1) * sizeof(char));
    id[0] = '\0';
    strncat(id, &l->contents[start], sizeof(char) * (l->index - start));

    return id;
}


char *lexer_collect_float(struct Lexer *l)
{
    size_t start = l->index;

    while ((isdigit(l->ch) || l->ch == '.') && l->ch != '\n' && l->ch != '\0')
        lexer_advance(l);

    char *s = malloc(sizeof(char) * (l->index - start + 1));
    s[0] = '\0';
    strncat(s, l->contents + start, sizeof(char) * (l->index - start));

    return s;
}


struct Token *lexer_next_token(struct Lexer *l)
{
    while (l->index < strlen(l->contents))
    {
        while (isspace(l->ch) && l->ch != '\n')
            lexer_advance(l);

        if (isdigit(l->ch))
        {
            size_t tmp_i = l->index;

            while (isdigit(l->contents[tmp_i]))
                ++tmp_i;

            if (l->contents[tmp_i] == '.')
                return token_alloc(TT_FLOAT, lexer_collect_float(l));
            else
                return token_alloc(TT_INT, lexer_collect_int(l));
        }

        if (isalnum(l->ch) || l->ch == '_')
            return token_alloc(TT_ID, lexer_collect_id(l));

        switch (l->ch)
        {
        case ';': return lexer_advance_with_token(l, token_alloc(TT_SEMI, strdup(";")));
        case '(': return lexer_advance_with_token(l, token_alloc(TT_LPAREN, strdup("(")));
        case ')': return lexer_advance_with_token(l, token_alloc(TT_RPAREN, strdup(")")));
        case '{': return lexer_advance_with_token(l, token_alloc(TT_LBRACE, strdup("{")));
        case '}': return lexer_advance_with_token(l, token_alloc(TT_RBRACE, strdup("}")));
        case '=': return lexer_advance_with_token(l, token_alloc(TT_EQUAL, strdup("=")));
        case ',': return lexer_advance_with_token(l, token_alloc(TT_COMMA, strdup(",")));
        case '.': return lexer_advance_with_token(l, token_alloc(TT_PERIOD, strdup(".")));
        case '+': return lexer_advance_with_token(l, token_binop_alloc(BT_PLUS, strdup("+")));
        case '\n':
            ++l->line_num;
            lexer_advance(l);
            break;
        default:
            fprintf(stderr, "Error in tokenizing shader on line %zu: Unrecognized character '%c'\n",
                    l->line_num, l->ch);
            exit(EXIT_FAILURE);
        }
    }

    return token_alloc(TT_EOF, strdup(""));
}


struct Token *lexer_advance_with_token(struct Lexer *l, struct Token *t)
{
    lexer_advance(l);
    return t;
}

