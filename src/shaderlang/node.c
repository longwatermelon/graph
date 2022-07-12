#include "node.h"
#include "visitor.h"
#include <stdlib.h>
#include <string.h>


struct Node *node_alloc(NodeType type)
{
    struct Node *n = malloc(sizeof(struct Node));
    n->type = type;

    n->vardef_name = 0;
    n->vardef_value = 0;
    n->vardef_modifier = VAR_REG;
    n->vardef_type = -1;
    n->vardef_layout_loc = -1;

    n->var_name = 0;

    n->call_name = 0;
    n->call_args = 0;
    n->call_nargs = 0;

    n->fdef_name = 0;
    n->fdef_body = 0;
    n->fdef_params = 0;
    n->fdef_nparams = 0;
    n->fdef_type = -1;

    n->param_type = -1;
    n->param_name = 0;

    n->assign_left = 0;
    n->assign_right = 0;

    n->construct_type = -1;
    n->construct_nvalues = 0;
    n->construct_values = 0;
    n->construct_out = 0;

    n->int_value = 0;

    n->comp_nvalues = 0;
    n->comp_value = 0;

    n->vec_values = 0;
    n->vec_len = 0;

    n->float_value = 0.f;

    return n;
}


void node_free(struct Node *n)
{
    if (!n) return;

    // Strings are copied so they must be freed
    free(n->param_name);
    free(n->fdef_name);
    free(n->call_name);
    free(n->var_name);
    free(n->vardef_name);

    if (n->vardef_value) node_free(n->vardef_value);
    if (n->assign_left) node_free(n->assign_left);
    if (n->assign_right) node_free(n->assign_right);
    if (n->fdef_body) node_free(n->fdef_body);
    if (n->construct_out) node_free(n->construct_out);

    if (n->call_args)
    {
        for (size_t i = 0; i < n->call_nargs; ++i)
            node_free(n->call_args[i]);

        free(n->call_args);
    }

    if (n->fdef_params)
    {
        for (size_t i = 0; i < n->fdef_nparams; ++i)
            node_free(n->fdef_params[i]);

        free(n->fdef_params);
    }

    if (n->comp_value)
    {
        for (size_t i = 0; i < n->comp_nvalues; ++i)
            node_free(n->comp_value[i]);

        free(n->comp_value);
    }

    if (n->construct_values)
    {
        for (size_t i = 0; i < n->construct_nvalues; ++i)
            node_free(n->construct_values[i]);

        free(n->construct_values);
    }

    if (n->vec_values)
    {
        for (size_t i = 0; i < n->vec_len; ++i)
            node_free(n->vec_values[i]);

        free(n->vec_values);
    }

    free(n);
}


struct Node *node_copy(struct Node *src)
{
    struct Node *n = node_alloc(src->type);

    switch (src->type)
    {
    case NODE_VEC:
    {
        n->vec_len = src->vec_len;
        n->vec_values = malloc(sizeof(struct Node*) * n->vec_len);

        for (size_t i = 0; i < n->vec_len; ++i)
            n->vec_values[i] = node_copy(src->vec_values[i]);
    } break;
    case NODE_FUNC_CALL:
    {
        n->call_name = strdup(src->call_name);
        n->call_nargs = src->call_nargs;
        n->call_args = malloc(sizeof(struct Node*) * n->call_nargs);

        for (size_t i = 0; i < n->call_nargs; ++i)
            n->call_args[i] = node_copy(src->call_args[i]);
    } break;
    case NODE_VAR:
    {
        n->var_name = strdup(src->var_name);
    } break;
    case NODE_VARDEF:
    {
        n->vardef_modifier = src->vardef_modifier;
        n->vardef_type = src->vardef_type;
        n->vardef_name = strdup(src->vardef_name);
        n->vardef_value = node_copy(src->vardef_value);
        n->vardef_layout_loc = src->vardef_layout_loc;
    } break;
    case NODE_INT:
    {
        n->int_value = src->int_value;
    } break;
    case NODE_COMPOUND:
    {
        n->comp_nvalues = src->comp_nvalues;
        n->comp_value = malloc(sizeof(struct Node*) * n->comp_nvalues);

        for (size_t i = 0; i < n->comp_nvalues; ++i)
            n->comp_value[i] = node_copy(src->comp_value[i]);

    } break;
    case NODE_FUNC_DEF:
    {
        n->fdef_type = src->fdef_type;
        n->fdef_body = node_copy(src->fdef_body);
        n->fdef_name = strdup(src->fdef_name);
        n->fdef_nparams = src->fdef_nparams;
        n->fdef_params = malloc(sizeof(struct Node*) * n->fdef_nparams);

        for (size_t i = 0; i < n->fdef_nparams; ++i)
            n->fdef_params[i] = node_copy(src->fdef_params[i]);

    } break;
    case NODE_ASSIGN:
    {
        n->assign_left = node_copy(src->assign_left);
        n->assign_right = node_copy(src->assign_right);
    } break;
    case NODE_PARAM:
    {
        n->param_name = strdup(src->param_name);
        n->param_type = src->param_type;
    } break;
    case NODE_CONSTRUCTOR:
    {
        n->construct_type = src->construct_type;
        n->construct_out = node_copy(src->construct_out);
        n->construct_nvalues = src->construct_nvalues;
        n->construct_values = malloc(sizeof(struct Node*) * n->construct_nvalues);

        for (size_t i = 0; i < n->construct_nvalues; ++i)
            n->construct_values[i] = node_copy(src->construct_values[i]);
    } break;
    case NODE_FLOAT:
    {
        n->float_value = src->float_value;
    } break;
    }

    return n;
}


int node_str2nt(const char *str)
{
    if (strcmp(str, "int") == 0) return NODE_INT;
    if (strncmp(str, "vec", 3) == 0) return NODE_VEC;
    if (strcmp(str, "void") == 0) return NODE_VOID;
    if (strcmp(str, "float") == 0) return NODE_FLOAT;

    return -1;
}


void node_to_vec(struct Node *vec, float *out)
{
    for (size_t i = 0; i < vec->vec_len; ++i)
        out[i] = visitor_visit(vec->vec_values[i])->float_value;
}

