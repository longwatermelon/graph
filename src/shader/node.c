#include "node.h"
#include <stdlib.h>
#include <string.h>


struct Node *node_alloc(int type)
{
    struct Node *n = malloc(sizeof(struct Node));
    n->type = type;

    n->vardef_name = 0;
    n->vardef_value = 0;
    n->vardef_modifier = VAR_REG;
    n->vardef_type = -1;

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

    n->int_value = 0;

    n->comp_nvalues = 0;
    n->comp_value = 0;

    glm_vec3_zero(n->vec3_value);

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

    free(n);
}


int node_str2nt(const char *str)
{
    if (strcmp(str, "int") == 0) return NODE_INT;
    if (strcmp(str, "vec3") == 0) return NODE_VEC3;
    if (strcmp(str, "void") == 0) return NODE_VOID;

    return -1;
}

