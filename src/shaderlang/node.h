#ifndef SHADER_NODE_H
#define SHADER_NODE_H

#include <sys/types.h>
#include <cglm/cglm.h>

typedef enum
{
    NODE_VARDEF,
    NODE_VAR,
    NODE_FUNC_CALL,
    NODE_FUNC_DEF,
    NODE_INT,
    NODE_FLOAT,
    NODE_VEC,
    NODE_VOID,
    NODE_COMPOUND,
    NODE_PARAM,
    NODE_ASSIGN,
    NODE_BINOP,
    NODE_CONSTRUCTOR
} NodeType;

typedef enum
{
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV
} Binop;

typedef enum
{
    VAR_REG,
    VAR_IN,
    VAR_OUT,
    VAR_LAYOUT
} VarModifier;

struct Node
{
    NodeType type;

    // vardef
    char *vardef_name;
    struct Node *vardef_value;
    VarModifier vardef_modifier;
    NodeType vardef_type;
    int vardef_layout_loc;

    // var
    char *var_name;
    char *var_memb_access; // Only allowing one layer of member access

    // func call
    char *call_name;
    struct Node **call_args;
    size_t call_nargs;

    // func def
    char *fdef_name;
    struct Node **fdef_params;
    size_t fdef_nparams;
    struct Node *fdef_body;
    NodeType fdef_type;

    // param
    NodeType param_type;
    char *param_name;

    // assign
    struct Node *assign_left, *assign_right;

    // constructor
    NodeType construct_type;
    struct Node *construct_out;

    // int
    int int_value;

    // compound
    struct Node **comp_value;
    size_t comp_nvalues;

    // vec3
    struct Node **vec_values;
    size_t vec_len;

    // float
    float float_value;

    // binop
    Binop op;
    struct Node *op_l, *op_r;
    struct Node *op_res; // Hack around no heap allocations in visitor
    bool op_priority;
};

struct Node *node_alloc(NodeType type);
void node_free(struct Node *n);

struct Node *node_copy(struct Node *src);

// String to node type
int node_str2nt(const char *str);

// Out: out
// Node parameter must be of type NODE_VEC
void node_to_vec(struct Node *vec, float *out);

#endif

