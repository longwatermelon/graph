#include "shader.h"
#include <string.h>


struct Shader *shader_alloc(const char *path)
{
    struct Shader *s = malloc(sizeof(struct Shader));
    strcpy(s->path, path);

    s->in = interp_alloc();

    struct Parser *parser = parser_alloc(path);
    s->root = parser_parse(parser);
    parser_free(parser);

    interp_prepare(s->in, s->root);

    return s;
}


void shader_free(struct Shader *s)
{
    interp_free(s->in);
    node_free(s->root);

    free(s);
}


void shader_run(struct Shader *s)
{
    interp_clear(s->in);

    interp_prepare(s->in, s->root);
    shader_insert_runtime_inputs(s, s->in);
    interp_run(s->in);

    shader_clear_inputs(s);
}


void shader_insert_runtime_inputs(struct Shader *s, struct Interpreter *in)
{
    for (size_t i = 0; i < in->nvardefs; ++i)
    {
        struct Node *def = in->vardefs[i];

        if (def->vardef_modifier == VAR_IN)
        {
            for (size_t j = 0; j < s->ninputs; ++j)
            {
                struct Node *input = s->inputs[j];

                if (strcmp(def->vardef_name, input->vardef_name) == 0)
                {
                    switch (def->vardef_type)
                    {
                    case NODE_INT:
                        def->vardef_value->int_value = input->vardef_value->int_value;
                        break;
                    case NODE_VEC:
                        for (size_t i = 0; i < def->vardef_value->vec_len; ++i)
                            node_free(def->vardef_value->vec_values[i]);

                        free(def->vardef_value->vec_values);

                        if (def->vardef_value->vec_len != input->vardef_value->vec_len)
                        {
                            def->vardef_value->vec_values = malloc(sizeof(struct Node*) * input->vardef_value->vec_len);
                            def->vardef_value->vec_len = input->vardef_value->vec_len;
                        }

                        for (size_t i = 0; i < input->vardef_value->vec_len; ++i)
                            def->vardef_value->vec_values[i] = node_copy(input->vardef_value->vec_values[i]);
                        break;
                    case NODE_FLOAT:
                        def->vardef_value->float_value = input->vardef_value->float_value;
                        break;
                    }

                    break;
                }
            }
        }
    }
}


void shader_add_input_int(struct Shader *s, const char *name, int i)
{
    struct Node *n = shader_new_input(s, name, NODE_INT);
    n->vardef_value = node_alloc(NODE_INT);
    n->vardef_value->int_value = i;
}


void shader_add_input_vec(struct Shader *s, const char *name, float *v, size_t len)
{
    struct Node *n = shader_new_input(s, name, NODE_VEC);
    n->vardef_value = node_alloc(NODE_VEC);

    n->vardef_value->vec_len = len;
    n->vardef_value->vec_values = malloc(sizeof(struct Node*) * len);

    for (size_t i = 0; i < len; ++i)
    {
        struct Node *num = node_alloc(NODE_FLOAT);
        num->float_value = v[i];
        n->vardef_value->vec_values[i] = num;
    }
}


void shader_add_input_float(struct Shader *s, const char *name, float f)
{
    struct Node *n = shader_new_input(s, name, NODE_FLOAT);
    n->vardef_value = node_alloc(NODE_FLOAT);
    n->vardef_value->float_value = f;
}


struct Node *shader_new_input(struct Shader *s, const char *name, int type)
{
    s->inputs = realloc(s->inputs, sizeof(struct Node*) * ++s->ninputs);
    s->inputs[s->ninputs - 1] = node_alloc(NODE_VARDEF);

    struct Node *n = s->inputs[s->ninputs - 1];
    n->vardef_type = type;
    n->vardef_name = strdup(name);
    n->vardef_modifier = VAR_REG;

    return n;
}


void shader_clear_inputs(struct Shader *s)
{
    for (size_t i = 0; i < s->ninputs; ++i)
        node_free(s->inputs[i]);

    free(s->inputs);

    s->inputs = 0;
    s->ninputs = 0;
}


struct Node *shader_outvar(struct Shader *s, const char *name)
{
    for (size_t i = 0; i < s->in->nvardefs; ++i)
    {
        struct Node *def = s->in->vardefs[i];

        if (strcmp(def->vardef_name, name) == 0 && def->vardef_modifier == VAR_OUT)
            return s->in->vardefs[i];
    }

    return 0;
}

