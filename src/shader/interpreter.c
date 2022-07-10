#include "interpreter.h"
#include <string.h>


struct Interpreter *interp_alloc()
{
    struct Interpreter *in = malloc(sizeof(struct Interpreter));
    in->vardefs = 0;
    in->nvardefs = 0;

    in->fdefs = 0;
    in->nfdefs = 0;

    return in;
}


void interp_free(struct Interpreter *in)
{
    // Interpreter copies nodes into vardefs and fdefs
    interp_clear(in);
    free(in);
}


void interp_run(struct Interpreter *in)
{
    struct Node *n = node_alloc(NODE_FUNC_CALL);
    n->call_name = strdup("main");
    interp_visit(in, n);
    node_free(n);
}


void interp_clear(struct Interpreter *in)
{
    for (size_t i = 0; i < in->nvardefs; ++i)
        node_free(in->vardefs[i]);

    free(in->vardefs);
    in->vardefs = 0;
    in->nvardefs = 0;

    for (size_t i = 0; i < in->nfdefs; ++i)
        node_free(in->fdefs[i]);

    free(in->fdefs);
    in->fdefs = 0;
    in->nfdefs = 0;
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
    case NODE_VEC3:
    case NODE_NOOP:
        return n;
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
    in->vardefs = realloc(in->vardefs, sizeof(struct Node*) * ++in->nvardefs);
    in->vardefs[in->nvardefs - 1] = node_copy(n);

    return n;
}


struct Node *interp_visit_var(struct Interpreter *in, struct Node *n)
{
    struct Node *def = interp_find_vardef(in, n->var_name);

    if (!def)
    {
        fprintf(stderr, "Interpreter error: Undefined variable '%s'.\n", n->var_name);
        exit(EXIT_FAILURE);
    }

    return interp_visit(in, def->vardef_value);
}


struct Node *interp_visit_call(struct Interpreter *in, struct Node *n)
{
    struct Node *def = interp_find_fdef(in, n->call_name);

    if (!def)
    {
        fprintf(stderr, "Interpreter error: Undefined function '%s'.\n", n->call_name);
        exit(EXIT_FAILURE);
    }

    return interp_visit(in, def->fdef_body);
}


struct Node *interp_visit_fdef(struct Interpreter *in, struct Node *n)
{
    in->fdefs = realloc(in->fdefs, sizeof(struct Node*) * ++in->nfdefs);
    in->fdefs[in->nfdefs - 1] = node_copy(n);

    return n;
}


struct Node *interp_visit_assignment(struct Interpreter *in, struct Node *n)
{
    struct Node *def = interp_find_vardef(in, n->assign_left->var_name);
    struct Node *right = interp_visit(in, n->assign_right);

    if (!def)
    {
        fprintf(stderr, "Interpreter error: Undefined variable '%s'.\n", def->vardef_name);
        exit(EXIT_FAILURE);
    }

    if (def->vardef_type != right->type)
    {
        fprintf(stderr, "Interpreter error: Mismatched types on '%s' assignment: '%d' and '%d'",
                def->vardef_name, def->vardef_type, right->type);
        exit(EXIT_FAILURE);
    }

    switch (def->vardef_type)
    {
    case NODE_INT: def->vardef_value->int_value = right->int_value; break;
    case NODE_VEC3: glm_vec3_copy(right->vec3_value, def->vardef_value->vec3_value); break;
    default:
        fprintf(stderr, "Interpreter error: %d is not a data type.\n", def->vardef_type);
        exit(EXIT_FAILURE);
    }

    if (n->assign_right->type == NODE_CONSTRUCTOR)
        node_free(right);

    return def;
}


struct Node *interp_visit_constructor(struct Interpreter *in, struct Node *n)
{
    switch (n->construct_type)
    {
    case NODE_INT:
    {
        if (n->construct_nvalues != 1)
        {
            fprintf(stderr, "Interpreter error: Expected 1 argument to int constructor but %zu were given.\n",
                    n->construct_nvalues);
            exit(EXIT_FAILURE);
        }

        struct Node *res = node_alloc(NODE_INT);
        res->int_value = interp_visit(in, n->construct_values[0])->int_value;
        return res;
    } break;
    case NODE_VEC3:
    {
        struct Node *res = node_alloc(NODE_VEC3);

        for (int i = 0; i < 3; ++i)
            res->vec3_value[i] = interp_visit(in, n->construct_values[i])->int_value;

        return res;
    } break;
    }

    fprintf(stderr, "Interpreter error: Constructor for type %d does not exist.\n", n->construct_type);
    exit(EXIT_FAILURE);
}


struct Node *interp_find_vardef(struct Interpreter *in, const char *name)
{
    for (size_t i = 0; i < in->nvardefs; ++i)
    {
        if (strcmp(in->vardefs[i]->vardef_name, name) == 0)
            return in->vardefs[i];
    }

    return 0;
}


struct Node *interp_find_fdef(struct Interpreter *in, const char *name)
{
    for (size_t i = 0; i < in->nfdefs; ++i)
    {
        if (strcmp(in->fdefs[i]->fdef_name, name) == 0)
            return in->fdefs[i];
    }

    return 0;
}


struct Node **interp_output_variables(struct Interpreter *in, size_t *n)
{
    struct Node **outputs = 0;
    *n = 0;

    for (size_t i = 0; i < in->nvardefs; ++i)
    {
        if (in->vardefs[i]->vardef_modifier == VAR_OUT)
        {
            outputs = realloc(outputs, sizeof(struct Node*) * ++*n);
            outputs[*n - 1] = node_copy(in->vardefs[i]);
        }
    }

    return outputs;
}

