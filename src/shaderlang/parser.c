#include "parser.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define IS_TYPE(str) (node_str2nt(str) != -1)

struct Parser *parser_alloc(const char *path)
{
    struct Parser *p = malloc(sizeof(struct Parser));
    p->lexer = lexer_alloc(util_read_file(path));
    p->curr = lexer_next_token(p->lexer);
    p->prev = 0;
    p->prev_node = 0;

    return p;
}


void parser_free(struct Parser *p)
{
    token_free(p->prev);
    token_free(p->curr);
    lexer_free(p->lexer);
    free(p);
}


void parser_expect(struct Parser *p, TokenType type)
{
    if (p->curr->type == type)
    {
        if (p->prev) token_free(p->prev);
        p->prev = p->curr;
        p->curr = lexer_next_token(p->lexer);
    }
    else
    {
        fprintf(stderr, "Parser error: Unexpected token '%s' of type %d; expected token of type %d.\n",
                p->curr->value, p->curr->type, type);
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
    bool found = true;

    switch (p->curr->type)
    {
    case TT_INT: p->prev_node = parser_parse_int(p); break;
    case TT_FLOAT: p->prev_node = parser_parse_float(p); break;
    case TT_ID: p->prev_node = parser_parse_id(p); break;
    default: found = false;
    }

    if (found)
    {
        if (p->curr->type == TT_BINOP)
        {
            struct Node *n = parser_parse_binop(p, p->prev_node);
            p->prev_node = n;
        }
    }
    else
    {
        p->prev_node = 0;
    }

    return p->prev_node;
}


struct Node *parser_parse_int(struct Parser *p)
{
    struct Node *n = node_alloc(NODE_INT);
    n->int_value = atoi(p->curr->value);
    parser_expect(p, TT_INT);

    return n;
}


struct Node *parser_parse_float(struct Parser *p)
{
    struct Node *n = node_alloc(NODE_FLOAT);
    n->float_value = atof(p->curr->value);
    parser_expect(p, TT_FLOAT);

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
    else if (strcmp(p->curr->value, "layout") == 0)
    {
        parser_expect(p, TT_ID);
        int loc = atoi(p->curr->value);
        parser_expect(p, TT_INT);

        struct Node *n = parser_parse_vardef(p);
        n->vardef_modifier = VAR_LAYOUT;
        n->vardef_layout_loc = loc;

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

    if (p->curr->type == TT_PERIOD)
    {
        parser_expect(p, TT_PERIOD);
        n->var_memb_access = strdup(p->curr->value);
        parser_expect(p, TT_ID);
    }

    return n;
}


struct Node *parser_parse_vardef(struct Parser *p)
{
    NodeType type = node_str2nt(p->curr->value);

    int vlen;
    if (type == NODE_VEC)
        vlen = p->curr->value[3] - '0';

    parser_expect(p, TT_ID);

    if (p->curr->type == TT_LPAREN)
        return parser_parse_constructor(p);

    char *name = strdup(p->curr->value);
    parser_expect(p, TT_ID);

    if (p->curr->type == TT_LPAREN)
        return parser_parse_fdef(p, type, name);

    struct Node *n = node_alloc(NODE_VARDEF);

    if (p->curr->type == TT_EQUAL)
    {
        parser_expect(p, TT_EQUAL);

        struct Node *value = parser_parse_expr(p);
        n->vardef_name = name;
        n->vardef_type = type;

        n->vardef_value = value;
    }
    else
    {
        n->vardef_name = name;
        n->vardef_type = type;

        // Default value allocation
        n->vardef_value = node_alloc(n->vardef_type);

        if (type == NODE_VEC)
        {
            n->vardef_value->vec_len = vlen;
            n->vardef_value->vec_values = malloc(sizeof(struct Node*) * vlen);

            for (size_t i = 0; i < vlen; ++i)
                n->vardef_value->vec_values[i] = node_alloc(NODE_FLOAT);
        }
    }

    return n;
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


struct Node *parser_parse_fdef(struct Parser *p, NodeType type, char *name)
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

    return n;
}


struct Node *parser_parse_constructor(struct Parser *p)
{
    // On lparen
    NodeType type = node_str2nt(p->prev->value);
    // For vec
    size_t expect_len = p->prev->value[3] - '0';
    char *name = strdup(p->prev->value);

    parser_expect(p, TT_LPAREN);

    struct Node *n = node_alloc(NODE_CONSTRUCTOR);
    n->construct_type = type;

    struct Node **values = 0;
    size_t nvalues = 0;

    while (p->curr->type != TT_RPAREN)
    {
        values = realloc(values, sizeof(struct Node*) * ++nvalues);
        values[nvalues - 1] = parser_parse_expr(p);

        if (p->curr->type != TT_RPAREN)
            parser_expect(p, TT_COMMA);
    }

    parser_expect(p, TT_RPAREN);

    switch (type)
    {
    case NODE_FLOAT:
    case NODE_INT:
    {
        n->construct_out = values[0];
    } break;
    case NODE_VEC:
    {
        if (expect_len != nvalues)
        {
            fprintf(stderr, "Parser error: Constructor of type '%s' does not take %zu args - "
                    "%zu were expected.\n",
                    name, nvalues, expect_len);
            exit(EXIT_FAILURE);
        }

        n->construct_out = node_alloc(NODE_VEC);
        n->construct_out->vec_len = nvalues;
        n->construct_out->vec_values = malloc(sizeof(struct Node*) * n->construct_out->vec_len);

        for (size_t i = 0; i < n->construct_out->vec_len; ++i)
            n->construct_out->vec_values[i] = values[i];
    } break;
    default:
        fprintf(stderr, "[parser_parse_constructor] Error: %d is not a data type.\n", type);
        exit(EXIT_FAILURE);
    }

    free(values);
    free(name);
    return n;
}


struct Node *parser_parse_binop(struct Parser *p, struct Node *left)
{
    struct Node *n = node_alloc(NODE_BINOP);

    switch (p->curr->value[0])
    {
    case '+': n->op = BINOP_ADD; break;
    default:
        fprintf(stderr, "[parser_parse_binop] Error: Operator '%c' not defined.\n", p->curr->value[0]);
        exit(EXIT_FAILURE);
    }

    parser_expect(p, TT_BINOP);

    n->op_l = left;

    struct Node *parent = parser_parse_expr(p);

    if (parent->type == NODE_BINOP)
    {
        struct Node *bot_left = parent;

        while (bot_left->op_l->type == NODE_BINOP)
            bot_left = bot_left->op_l;

        n->op_r = bot_left->op_l;
        bot_left->op_l = n;

        return parent;
    }
    else
    {
        n->op_r = parent;
        return n;
    }
}

