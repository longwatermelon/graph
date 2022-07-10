#ifndef SHADER_NODE_H
#define SHADER_NODE_H

#include <sys/types.h>
#include <cglm/cglm.h>

enum
{
    NODE_VARDEF,
    NODE_VAR,
    NODE_FUNC_CALL,
    NODE_FUNC_DEF,
    NODE_INT,
    NODE_VEC3,
    NODE_VOID,
    NODE_COMPOUND,
    NODE_PARAM,
    NODE_ASSIGN,
    NODE_CONSTRUCTOR
};

enum
{
    VAR_REG,
    VAR_IN,
    VAR_OUT
};

struct Node
{
    int type;

    // vardef
    char *vardef_name;
    struct Node *vardef_value;
    int vardef_modifier;
    int vardef_type;

    // var
    char *var_name;

    // func call
    char *call_name;
    struct Node **call_args;
    size_t call_nargs;

    // func def
    char *fdef_name;
    struct Node **fdef_params;
    size_t fdef_nparams;
    struct Node *fdef_body;
    int fdef_type;

    // param
    int param_type;
    char *param_name;

    // assign
    struct Node *assign_left, *assign_right;

    // constructor
    int construct_type;
    struct Node **construct_values;
    size_t construct_nvalues;

    // int
    int int_value;

    // compound
    struct Node **comp_value;
    size_t comp_nvalues;

    // vec3
    vec3 vec3_value;
};

struct Node *node_alloc(int type);
void node_free(struct Node *n);

struct Node *node_copy(struct Node *src);

// String to node type
int node_str2nt(const char *str);

#endif

