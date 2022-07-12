#include "shader.h"
#include "buffer.h"
#include "attrib.h"
#include "render.h"
#include <string.h>

struct VertFragInfo *vfi_alloc(struct Shader *s)
{
    struct VertFragInfo *vfi = malloc(sizeof(struct VertFragInfo));
    vfi->outputs = scope_get_vardef_type(s->scope_vert, VAR_OUT, true, &vfi->len);

    struct Node *pos_node = shader_outvar(s, s->scope_vert, "gr_pos");
    node_to_vec(pos_node->vardef_value, vfi->pos);

    return vfi;
}

void vfi_free(struct VertFragInfo *vfi)
{
    for (size_t i = 0; i < vfi->len; ++i)
        node_free(vfi->outputs[i]);

    free(vfi->outputs);
    free(vfi);
}

struct Shader *shader_alloc(const char *vert, const char *frag)
{
    struct Shader *s = malloc(sizeof(struct Shader));

    s->inputs = 0;
    s->ninputs = 0;

    s->scope_vert = scope_alloc();
    s->scope_frag = scope_alloc();

    struct Parser *parser_vert = parser_alloc(vert);
    s->root_vert = parser_parse(parser_vert);
    parser_free(parser_vert);

    struct Parser *parser_frag = parser_alloc(frag);
    s->root_frag = parser_parse(parser_frag);
    parser_free(parser_frag);

    visitor_bind_scope(s->scope_vert);
    visitor_visit(s->root_vert);

    visitor_bind_scope(s->scope_frag);
    visitor_visit(s->root_frag);

    s->main_call = node_alloc(NODE_FUNC_CALL);
    s->main_call->call_name = strdup("main");

    return s;
}


void shader_free(struct Shader *s)
{
    scope_free(s->scope_vert);
    node_free(s->root_vert);

    scope_free(s->scope_frag);
    node_free(s->root_frag);

    node_free(s->main_call);

    free(s);
}


void shader_run_vert(struct Shader *s, SDL_Renderer *rend)
{
    struct Buffer *buf = graph_buffer_bound();
    struct AttribLayout *atl = graph_atl_bound();

    float *start = buf->data;

    size_t i = 0;
    while (i < buf->data_len)
    {
        struct VertFragInfo *verts[3];

        for (int j = 0; j < 3; ++j)
        {
            // Vertex shader
            visitor_bind_scope(s->scope_vert);
            scope_clear(s->scope_vert);
            visitor_visit(s->root_vert);

            shader_insert_layout_vars(s, start);
            shader_insert_runtime_inputs(s);
            visitor_visit(s->main_call);

            verts[j] = vfi_alloc(s);
            start += atl->stride;
        }

        graph_render_draw_tri(rend, verts);

        for (int j = 0; j < 3; ++j)
            vfi_free(verts[j]);

        i += atl->stride * 3;
    }

    shader_clear_inputs(s);
}


void shader_run_frag(struct Shader *s)
{
    // Inputs are already refreshed, no need to clear

    visitor_bind_scope(s->scope_frag);
    scope_clear(s->scope_frag);
    visitor_visit(s->root_frag);

    shader_insert_runtime_inputs(s);
    visitor_visit(s->main_call);
}


void shader_insert_runtime_inputs(struct Shader *s)
{
    struct Scope *scope = visitor_scope_bound();
    struct ScopeLayer *layer = &scope->layers[0];

    for (size_t i = 0; i < layer->nvardefs; ++i)
    {
        struct Node *def = layer->vardefs[i];

        if (def->vardef_modifier == VAR_IN)
        {
            for (size_t j = 0; j < s->ninputs; ++j)
            {
                struct Node *input = s->inputs[j];

                if (strcmp(def->vardef_name, input->vardef_name) == 0)
                {
                    node_free(def->vardef_value);
                    def->vardef_value = node_copy(input->vardef_value);

                    break;
                }
            }
        }
    }
}


void shader_insert_layout_vars(struct Shader *s, float *start)
{
    struct Scope *scope = visitor_scope_bound();
    struct ScopeLayer *layer = &scope->layers[0];

    struct AttribLayout *atl = graph_atl_bound();

    for (size_t i = 0; i < layer->nvardefs; ++i)
    {
        struct Node *def = layer->vardefs[i];

        if (def->vardef_modifier == VAR_LAYOUT)
        {
            struct AttribLayoutElement *atle = &atl->layout[def->vardef_layout_loc];
            float *begin = start + atle->offset;

            if (def->vardef_value->vec_len != atle->count)
            {
                fprintf(stderr, "[shader_insert_layout_vars] Error: Variable vector length (%zu) "
                        "does not match attrib layout count (%d).\n", def->vardef_value->vec_len, atle->count);
                exit(EXIT_FAILURE);
            }

            for (int j = 0; j < atle->count; ++j)
            {
                struct Node *n = node_alloc(NODE_FLOAT);
                n->float_value = begin[j];

                node_free(def->vardef_value->vec_values[j]);
                def->vardef_value->vec_values[j] = n;
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


struct Node *shader_outvar(struct Shader *s, struct Scope *scope, const char *name)
{
    struct ScopeLayer *layer = &scope->layers[0];

    for (size_t i = 0; i < layer->nvardefs; ++i)
    {
        struct Node *def = layer->vardefs[i];

        if (strcmp(def->vardef_name, name) == 0 && def->vardef_modifier == VAR_OUT)
            return layer->vardefs[i];
    }

    return 0;
}

