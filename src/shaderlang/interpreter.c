#include "interpreter.h"
#include <string.h>


struct Interpreter *interp_alloc()
{
    struct Interpreter *in = malloc(sizeof(struct Interpreter));
    in->scope = scope_alloc();

    return in;
}


void interp_free(struct Interpreter *in)
{
    scope_free(in->scope);
    free(in);
}


void interp_run(struct Interpreter *in)
{
    struct Node *n = node_alloc(NODE_FUNC_CALL);
    n->call_name = strdup("main");
    interp_visit(in, n);
    node_free(n);
}


void interp_prepare(struct Interpreter *in, struct Node *root)
{
    interp_visit(in, root);
}


struct Node *interp_visit(struct Interpreter *in, struct Node *n)
{
    if (!n) return 0;

    switch (n->type)
    {
    case NODE_INT:
    case NODE_VOID:
    case NODE_FLOAT:
        return n;
    case NODE_VEC: return interp_visit_vec(in, n);
    case NODE_COMPOUND: return interp_visit_compound(in, n);
    case NODE_VAR: return interp_visit_var(in, n);
    case NODE_VARDEF: return interp_visit_vardef(in, n);
    case NODE_FUNC_CALL: return interp_visit_call(in, n);
    case NODE_FUNC_DEF: return interp_visit_fdef(in, n);
    case NODE_ASSIGN: return interp_visit_assignment(in, n);
    case NODE_CONSTRUCTOR: return interp_visit_constructor(in, n);
    }

    fprintf(stderr, "Interpreter error: Uncaught statement of node type %d.\n",
            n->type);
    exit(EXIT_FAILURE);
}


struct Node *interp_visit_compound(struct Interpreter *in, struct Node *n)
{
    for (size_t i = 0; i < n->comp_nvalues; ++i)
        interp_visit(in, n->comp_value[i]);

    return n;
}


struct Node *interp_visit_vardef(struct Interpreter *in, struct Node *n)
{
    scope_add_vardef(in->scope, node_copy(n));
    return n;
}


struct Node *interp_visit_var(struct Interpreter *in, struct Node *n)
{
    struct Node *def = scope_find_vardef(in->scope, n->var_name, true);
    return interp_visit(in, def->vardef_value);
}


struct Node *interp_visit_call(struct Interpreter *in, struct Node *n)
{
    struct Node *def = scope_find_fdef(in->scope, n->call_name, true);
    return interp_visit(in, def->fdef_body);
}


struct Node *interp_visit_fdef(struct Interpreter *in, struct Node *n)
{
    scope_add_fdef(in->scope, node_copy(n));
    return n;
}


struct Node *interp_visit_assignment(struct Interpreter *in, struct Node *n)
{
    struct Node *def = scope_find_vardef(in->scope, n->assign_left->var_name, true);
    struct Node *right = interp_visit(in, n->assign_right);

    if (def->vardef_type != right->type)
    {
        fprintf(stderr, "Interpreter error: Mismatched types on '%s' assignment: '%d' and '%d'",
                def->vardef_name, def->vardef_type, right->type);
        exit(EXIT_FAILURE);
    }

    switch (def->vardef_type)
    {
    case NODE_INT: def->vardef_value->int_value = right->int_value; break;
    case NODE_VEC:
        node_free(def->vardef_value);
        def->vardef_value = node_copy(right);
        break;
    case NODE_FLOAT: def->vardef_value->float_value = right->float_value; break;
    default:
        fprintf(stderr, "Interpreter error: %d is not a data type.\n", def->vardef_type);
        exit(EXIT_FAILURE);
    }

    return def;
}


struct Node *interp_visit_constructor(struct Interpreter *in, struct Node *n)
{
    return interp_visit(in, n->construct_out);
}


struct Node *interp_visit_vec(struct Interpreter *in, struct Node *n)
{
    for (size_t i = 0; i < n->vec_len; ++i)
    {
        struct Node *tmp = node_copy(interp_visit(in, n->vec_values[i]));
        node_free(n->vec_values[i]);
        n->vec_values[i] = tmp;
    }

    return n;
}


struct Node **interp_output_variables(struct Interpreter *in, size_t *n)
{
    struct Node **outputs = 0;
    *n = 0;

    struct ScopeLayer *layer = &in->scope->layers[0];

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

