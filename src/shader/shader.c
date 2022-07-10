#include "shader.h"
#include <string.h>


struct Shader *shader_alloc(const char *vert, const char *frag)
{
    struct Shader *s = malloc(sizeof(struct Shader));
    strcpy(s->vert, vert);
    strcpy(s->frag, frag);

    s->vert_in = interp_alloc();
    s->frag_in = interp_alloc();

    struct Parser *parser = parser_alloc(vert);
    s->vert_root = parser_parse(parser);
    parser_free(parser);

    parser = parser_alloc(frag);
    s->frag_root = parser_parse(parser);
    parser_free(parser);

    interp_prepare(s->vert_in, s->vert_root);
    interp_prepare(s->frag_in, s->frag_root);

    return s;
}


void shader_free(struct Shader *s)
{
    interp_free(s->vert_in);
    interp_free(s->frag_in);

    node_free(s->vert_root);
    node_free(s->frag_root);

    free(s);
}


void shader_run(struct Shader *s)
{
    interp_clear(s->vert_in);
    interp_clear(s->frag_in);

    interp_prepare(s->vert_in, s->vert_root);
    shader_insert_runtime_inputs(s, s->vert_in);
    interp_run(s->vert_in);

    // Send outputs from vertex shader to fragment shader
    shader_clear_inputs(s);
    s->inputs = interp_output_variables(s->vert_in, &s->ninputs);

    interp_prepare(s->frag_in, s->frag_root);
    shader_insert_runtime_inputs(s, s->frag_in);
    interp_run(s->frag_in);

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
                    case NODE_VEC3:
                        glm_vec3_copy(input->vardef_value->vec3_value, def->vardef_value->vec3_value);
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


void shader_add_input_vec3(struct Shader *s, const char *name, vec3 v)
{
    struct Node *n = shader_new_input(s, name, NODE_VEC3);
    n->vardef_value = node_alloc(NODE_VEC3);
    glm_vec3_copy(v, n->vardef_value->vec3_value);
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


struct Node *shader_frag_outvar(struct Shader *s, const char *name)
{
    for (size_t i = 0; i < s->frag_in->nvardefs; ++i)
    {
        if (strcmp(s->frag_in->vardefs[i]->vardef_name, name) == 0)
            return s->frag_in->vardefs[i];
    }

    return 0;
}

