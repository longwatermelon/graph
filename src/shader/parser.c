#include "parser.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IS_TYPE(str) (strcmp(str, "vec3") == 0 || strcmp(str, "int") == 0 || strcmp(str, "void") == 0)

struct Parser *parser_alloc(const char *path)
{
    struct Parser *p = malloc(sizeof(struct Parser));
    p->lexer = lexer_alloc(util_read_file(path));
    p->curr = lexer_next_token(p->lexer);
    p->prev = 0;

    return p;
}


void parser_free(struct Parser *p)
{
    lexer_free(p->lexer);
}


void parser_expect(struct Parser *p, int ttype)
{
    if (p->curr->type == ttype)
    {
        if (p->prev) token_free(p->prev);
        p->prev = p->curr;
        p->curr = lexer_next_token(p->lexer);
    }
    else
    {
        fprintf(stderr, "Parser error: Unexpected token '%s' of type %d; expected token of type %d.\n",
                p->curr->value, p->curr->type, ttype);
        exit(EXIT_FAILURE);
    }
}


struct Node *parser_parse(struct Parser *p)
{
    struct Node *root = node_alloc(NODE_COMPOUND);
    root->comp_value = malloc(sizeof(struct Node*));
    root->comp_value[0] = parser_parse_expr(p);
    ++root->comp_nvalues;

    while (p->lexer->index < strlen(p->lexer->contents))
    {
        parser_expect(p, TT_SEMI);

        struct Node *expr = parser_parse_expr(p);
        if (!expr) break;

        root->comp_value = realloc(root->comp_value, sizeof(struct Node) * ++root->comp_nvalues);
        root->comp_value[root->comp_nvalues - 1] = expr;
    }

    return root;
}


struct Node *parser_parse_expr(struct Parser *p)
{
    switch (p->curr->type)
    {
    case TT_INT: return parser_parse_int(p);
    case TT_ID: return parser_parse_id(p);
    }

    return 0;
}


struct Node *parser_parse_int(struct Parser *p)
{
    struct Node *n = node_alloc(NODE_INT);
    n->int_value = atoi(p->curr->value);
    parser_expect(p, TT_INT);

    return n;
}


struct Node *parser_parse_id(struct Parser *p)
{
    if (strcmp(p->curr->value, "in") == 0)
    {
        parser_expect(p, TT_ID);
        struct Node *n = parser_parse_vardef(p);
        n->vardef_modifier = VAR_IN;
        return n;
    }
    else if (strcmp(p->curr->value, "out") == 0)
    {
        parser_expect(p, TT_ID);
        struct Node *n = parser_parse_vardef(p);
        n->vardef_modifier = VAR_OUT;
        return n;
    }

    if (IS_TYPE(p->curr->value))
        return parser_parse_vardef(p);
    else
        return parser_parse_var(p);
}


struct Node *parser_parse_var(struct Parser *p)
{
    // No strdup, early return leaks memory
    char *id = p->curr->value;
    parser_expect(p, TT_ID);

    if (p->curr->type == TT_LPAREN) return parser_parse_call(p);
    if (p->curr->type == TT_EQUAL) return parser_parse_assign(p);

    struct Node *n = node_alloc(NODE_VAR);
    n->var_name = strdup(id);

    return n;
}


struct Node *parser_parse_vardef(struct Parser *p)
{
    int type = node_str2nt(p->curr->value);
    parser_expect(p, TT_ID);

    if (p->curr->type == TT_LPAREN)
        return parser_parse_constructor(p);

    char *name = strdup(p->curr->value);
    parser_expect(p, TT_ID);

    if (p->curr->type == TT_EQUAL)
    {
        parser_expect(p, TT_EQUAL);

        struct Node *value = parser_parse_expr(p);
        struct Node *n = node_alloc(NODE_VARDEF);

        n->vardef_name = name;
        n->vardef_type = type;

        n->vardef_value = value;

        return n;
    }
    else if (p->curr->type == TT_LPAREN)
    {
        return parser_parse_fdef(p, type, name);
    }
    else
    {
        struct Node *n = node_alloc(NODE_VARDEF);
        n->vardef_name = name;
        n->vardef_type = type;
        // Default value allocation
        n->vardef_value = node_alloc(n->vardef_type);

        return n;
    }
}


struct Node *parser_parse_call(struct Parser *p)
{
    struct Node *n = node_alloc(NODE_FUNC_CALL);
    n->call_name = p->prev->value;

    parser_expect(p, TT_LPAREN);

    // Function args
    while (p->curr->type != TT_RPAREN)
    {
        struct Node *expr = parser_parse_expr(p);

        n->call_args = realloc(n->call_args, sizeof(struct Node*) * ++n->call_nargs);
        n->call_args[n->call_nargs - 1] = expr;

        if (p->curr->type != TT_RPAREN)
            parser_expect(p, TT_COMMA);
    }

    parser_expect(p, TT_RPAREN);
    return n;
}


struct Node *parser_parse_fdef(struct Parser *p, int type, char *name)
{
    struct Node *n = node_alloc(NODE_FUNC_DEF);
    n->fdef_type = type;
    n->fdef_name = name;

    // Parameters
    parser_expect(p, TT_LPAREN);

    while (p->curr->type != TT_RPAREN)
    {
        struct Node *param = node_alloc(NODE_PARAM);

        param->param_type = node_str2nt(p->curr->value);
        parser_expect(p, TT_ID);

        param->param_name = strdup(p->curr->value);
        parser_expect(p, TT_ID);

        n->fdef_params = realloc(n->fdef_params, sizeof(struct Node*) * ++n->fdef_nparams);
        n->fdef_params[n->fdef_nparams - 1] = param;

        if (p->curr->type != TT_RPAREN)
            parser_expect(p, TT_COMMA);
    }

    parser_expect(p, TT_RPAREN);

    // Function body
    parser_expect(p, TT_LBRACE);
    n->fdef_body = parser_parse(p);
    parser_expect(p, TT_RBRACE);

    return n;
}


struct Node *parser_parse_assign(struct Parser *p)
{
    // On equal
    struct Node *n = node_alloc(NODE_ASSIGN);

    n->assign_left = node_alloc(NODE_VAR);
    n->assign_left->var_name = strdup(p->prev->value);

    parser_expect(p, TT_EQUAL);

    n->assign_right = parser_parse_expr(p);
    /* n->assign_right = node_alloc(NODE_VAR); */
    /* n->assign_right->var_name = strdup(p->curr->value); */

    return n;
}


struct Node *parser_parse_constructor(struct Parser *p)
{
    // On lparen
    int type = node_str2nt(p->prev->value);
    parser_expect(p, TT_LPAREN);

    struct Node *n = node_alloc(NODE_CONSTRUCTOR);
    n->construct_type = type;

    while (p->curr->type != TT_RPAREN)
    {
        n->construct_values = realloc(n->construct_values, sizeof(struct Node*) * ++n->construct_nvalues);
        n->construct_values[n->construct_nvalues - 1] = parser_parse_expr(p);

        if (p->curr->type != TT_RPAREN)
            parser_expect(p, TT_COMMA);
    }

    parser_expect(p, TT_RPAREN);
    return n;

/*     switch (type) */
/*     { */
/*     case NODE_INT: */
/*     { */
/*         struct Node *n = parser_parse_int(p); */
/*         parser_expect(p, TT_RPAREN); */
/*         return n; */
/*     } break; */
/*     case NODE_VEC3: */
/*     { */
/*         struct Node *n = node_alloc(NODE_VEC3); */

/*         n->vec3_value[0] = atoi(p->curr->value); */
/*         parser_expect(p, TT_INT); */
/*         parser_expect(p, TT_COMMA); */

/*         n->vec3_value[1] = atoi(p->curr->value); */
/*         parser_expect(p, TT_INT); */
/*         parser_expect(p, TT_COMMA); */

/*         n->vec3_value[2] = atoi(p->curr->value); */
/*         parser_expect(p, TT_INT); */
/*         parser_expect(p, TT_RPAREN); */

/*         return n; */
/*     } break; */
/*     case NODE_VOID: */
/*         fprintf(stderr, "Error: Can't construct variable of type void.\n"); */
/*         exit(EXIT_FAILURE); */
/*     } */

/*     fprintf(stderr, "Unrecognized constructor type '%s'.\n", p->prev->value); */
/*     exit(EXIT_FAILURE); */
}

