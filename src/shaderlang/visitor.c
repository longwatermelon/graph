#include "visitor.h"
#include <string.h>

struct Scope *g_scope = 0;

struct Node *visitor_visit(struct Node *n)
{
    if (!n) return 0;

    switch (n->type)
    {
    case NODE_INT:
    case NODE_VOID:
    case NODE_FLOAT:
        return n;
    case NODE_VEC: return visitor_visit_vec(n);
    case NODE_COMPOUND: return visitor_visit_compound(n);
    case NODE_VAR: return visitor_visit_var(n);
    case NODE_VARDEF: return visitor_visit_vardef(n);
    case NODE_FUNC_CALL: return visitor_visit_call(n);
    case NODE_FUNC_DEF: return visitor_visit_fdef(n);
    case NODE_ASSIGN: return visitor_visit_assignment(n);
    case NODE_CONSTRUCTOR: return visitor_visit_constructor(n);
    case NODE_PARAM: return visitor_visit_param(n);
    case NODE_BINOP: return visitor_visit_binop(n);
    }

    fprintf(stderr, "Interpreter error: Uncaught statement of node type %d.\n",
            n->type);
    exit(EXIT_FAILURE);
}


struct Node *visitor_visit_compound(struct Node *n)
{
    for (size_t i = 0; i < n->comp_nvalues; ++i)
        visitor_visit(n->comp_value[i]);

    return n;
}


struct Node *visitor_visit_vardef(struct Node *n)
{
    scope_add_vardef(g_scope, node_copy(n));
    return n;
}


struct Node *visitor_visit_var(struct Node *n)
{
    struct Node *def = scope_find_vardef(g_scope, n->var_name, true);
    return visitor_visit(def->vardef_value);
}


struct Node *visitor_visit_call(struct Node *n)
{
    struct Node *def = scope_find_fdef(g_scope, n->call_name, true);
    return visitor_visit(def->fdef_body);
}


struct Node *visitor_visit_fdef(struct Node *n)
{
    scope_add_fdef(g_scope, node_copy(n));
    return n;
}


struct Node *visitor_visit_assignment(struct Node *n)
{
    struct Node *def = scope_find_vardef(g_scope, n->assign_left->var_name, true);
    struct Node *right = visitor_visit(n->assign_right);

    if (def->vardef_type != right->type)
    {
        fprintf(stderr, "Interpreter error: Mismatched types on '%s' assignment: '%d' and '%d'",
                def->vardef_name, def->vardef_type, right->type);
        exit(EXIT_FAILURE);
    }

    node_free(def->vardef_value);
    def->vardef_value = node_copy(right);

    return def;
}


struct Node *visitor_visit_constructor(struct Node *n)
{
    return visitor_visit(n->construct_out);
}


struct Node *visitor_visit_vec(struct Node *n)
{
    for (size_t i = 0; i < n->vec_len; ++i)
    {
        struct Node *tmp = node_copy(visitor_visit(n->vec_values[i]));
        node_free(n->vec_values[i]);
        n->vec_values[i] = tmp;
    }

    return n;
}


struct Node *visitor_visit_param(struct Node *n)
{
    struct Node *def = scope_find_vardef(g_scope, n->param_name, true);
    return visitor_visit(def->vardef_value);
}


struct Node *visitor_visit_binop(struct Node *n)
{
    struct Node *l = visitor_visit(n->op_l);
    struct Node *r = visitor_visit(n->op_r);

    if (l->type != r->type)
    {
        fprintf(stderr, "[visitor_visit_binop] Error: Types %d and %d are incompatible.\n",
                l->type, r->type);
        exit(EXIT_FAILURE);
    }

    n->op_res->type = l->type;

    switch (l->type)
    {
    case NODE_FLOAT:
        n->op_res->float_value = l->float_value + r->float_value;
        break;
    case NODE_INT:
        n->op_res->int_value = l->int_value + r->int_value;
        break;
    default:
        fprintf(stderr, "[visitor_visit_binop] Error: %d is not a data type.\n",
                l->type);
        exit(EXIT_FAILURE);
    }

    return n->op_res;
}


struct Node **visitor_output_variables(size_t *n)
{
    struct Node **outputs = 0;
    *n = 0;

    struct ScopeLayer *layer = &g_scope->layers[0];

    for (size_t i = 0; i < layer->nvardefs; ++i)
    {
        if (layer->vardefs[i]->vardef_modifier == VAR_OUT)
        {
            outputs = realloc(outputs, sizeof(struct Node*) * ++*n);
            outputs[*n - 1] = node_copy(layer->vardefs[i]);
        }
    }

    return outputs;
}


void visitor_bind_scope(struct Scope *s)
{
    g_scope = s;
}


struct Scope *visitor_scope_bound()
{
    return g_scope;
}

