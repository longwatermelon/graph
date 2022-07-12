#ifndef SHADER_PARSER_H
#define SHADER_PARSER_H

#include "token.h"
#include "lexer.h"
#include "node.h"

struct Parser
{
    struct Token *curr, *prev;
    struct Lexer *lexer;

    struct Node *prev_node;
};

struct Parser *parser_alloc(const char *path);
void parser_free(struct Parser *p);

void parser_expect(struct Parser *p, TokenType type);

struct Node *parser_parse(struct Parser *p);
struct Node *parser_parse_expr(struct Parser *p);

struct Node *parser_parse_int(struct Parser *p);
struct Node *parser_parse_float(struct Parser *p);
struct Node *parser_parse_id(struct Parser *p);

struct Node *parser_parse_var(struct Parser *p);
struct Node *parser_parse_vardef(struct Parser *p);
struct Node *parser_parse_call(struct Parser *p);
struct Node *parser_parse_fdef(struct Parser *p, NodeType type, char *name);
struct Node *parser_parse_assign(struct Parser *p);
struct Node *parser_parse_constructor(struct Parser *p);
struct Node *parser_parse_binop(struct Parser *p, struct Node *left);

#endif

