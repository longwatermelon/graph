#include "render.h"
#include "buffer.h"
#include "shader.h"
#include "util.h"

#define swapv(v1, v2, a, b) { \
    struct VertFragInfo *tmp = v1; \
    v1 = v2; \
    v2 = tmp; \
    vec3 tmpv; \
    glm_vec3_copy(a, tmpv); \
    glm_vec3_copy(b, a); \
    glm_vec3_copy(tmpv, b); \
}

int g_w = 0, g_h = 0;

uint32_t *g_scr = 0;
float *g_zbuf = 0;

struct Shader *g_shader = 0;

void graph_init_renderer(int w, int h)
{
    g_scr = malloc(sizeof(uint32_t) * (w * h));
    g_zbuf = malloc(sizeof(float) * (w * h));
    g_w = w;
    g_h = h;
}

void graph_free_renderer()
{
    free(g_scr);
    free(g_zbuf);
}

void graph_reset_renderer()
{
    for (int i = 0; i < g_w * g_h; ++i)
    {
        g_scr[i] = 0x00000000;
        g_zbuf[i] = INFINITY;
    }
}

void graph_render_result(SDL_Texture *tex)
{
    SDL_UpdateTexture(tex, 0, g_scr, g_w * sizeof(uint32_t));
}

void graph_draw(SDL_Renderer *rend)
{
    graph_reset_renderer();
    shader_run_vert(g_shader, rend);
}

void graph_use_shader(struct Shader *s)
{
    g_shader = s;
}

void graph_render_draw_tri(SDL_Renderer *rend, struct VertFragInfo *points[3])
{
    vec3 a, b, c;
    glm_vec3_copy(points[0]->pos, a);
    glm_vec3_copy(points[1]->pos, b);
    glm_vec3_copy(points[2]->pos, c);

    if (a[1] > b[1]) swapv(points[0], points[1], a, b);
    if (a[1] > c[1]) swapv(points[0], points[2], a, c);
    if (b[1] > c[1]) swapv(points[1], points[2], b, c);

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

    RTI r_ac = { a[0], (c[1] - a[1]) / (c[0] - a[0]), a[2], (c[2] - a[2]) / (c[1] - a[1]) };
    RTI r_ab = { a[0], (b[1] - a[1]) / (b[0] - a[0]), a[2], (b[2] - a[2]) / (b[1] - a[1]) };
    RTI r_bc = { b[0], (c[1] - b[1]) / (c[0] - b[0]), b[2], (c[2] - b[2]) / (c[1] - b[1]) };

    fill_edges(points[0], points[1], &r_ac, &r_ab, points);
    fill_edges(points[1], points[2], &r_ac, &r_bc, points);
}

void interp_vec(struct VertFragInfo *verts[3], vec3 bary, size_t i)
{
    // a * ba + b * bb + c * bc
    vec3 aba;
    node_to_vec(verts[0]->outputs[i]->vardef_value, aba);
    glm_vec3_scale(aba, bary[0], aba);

    vec3 bbb;
    node_to_vec(verts[1]->outputs[i]->vardef_value, bbb);
    glm_vec3_scale(bbb, bary[1], bbb);

    vec3 cbc;
    node_to_vec(verts[2]->outputs[i]->vardef_value, cbc);
    glm_vec3_scale(cbc, bary[2], cbc);

    vec3 v;
    glm_vec3_add(aba, bbb, v);
    glm_vec3_add(v, cbc, v);

    shader_add_input_vec(g_shader, verts[0]->outputs[i]->vardef_name, v, 3);
}

void interp_float(struct VertFragInfo *verts[3], vec3 bary, size_t i)
{
    struct Node *a = visitor_visit(verts[0]->outputs[i]->vardef_value);
    struct Node *b = visitor_visit(verts[1]->outputs[i]->vardef_value);
    struct Node *c = visitor_visit(verts[2]->outputs[i]->vardef_value);

    float fa = a->float_value;
    float fb = b->float_value;
    float fc = c->float_value;

    float f = fa * bary[0] + fb * bary[1] + fc * bary[2];
    shader_add_input_float(g_shader, verts[0]->outputs[i]->vardef_name, f);
}

void fill_edges(struct VertFragInfo *va, struct VertFragInfo *vb, RTI *l1, RTI *l2, struct VertFragInfo *verts[3])
{
    vec3 a, b;
    glm_vec3_copy(va->pos, a);
    glm_vec3_copy(vb->pos, b);

    vec3 positions[3];
    glm_vec3_copy(verts[0]->pos, positions[0]);
    glm_vec3_copy(verts[1]->pos, positions[1]);
    glm_vec3_copy(verts[2]->pos, positions[2]);

    for (int y = a[1]; y < b[1]; ++y)
    {
        int min = roundf(l1->x < l2->x ? l1->x : l2->x);
        int max = roundf(l1->x > l2->x ? l1->x : l2->x);

        float z = l1->z;
        float sz = (l2->z - l1->z) / (l2->x - l1->x);

        for (int i = min; i < max; ++i)
        {
            z += sz;

            if (i < 0)
            {
                z += sz * -i;
                i = 0;
                continue;
            }

            if (i >= g_w) break;

            vec3 pos = { i, y, z };
            vec3 bary;
            util_bary_coefficients(positions, pos, bary);

            visitor_bind_scope(g_shader->scope_frag);
            shader_clear_inputs(g_shader);

            // Arbitrarily chose va for iterator condition since they
            // should all have the same length anyways
            for (size_t i = 0; i < va->len; ++i)
            {
                switch (va->outputs[i]->vardef_type)
                {
                case NODE_VEC: interp_vec(verts, bary, i); break;
                case NODE_FLOAT: interp_float(verts, bary, i); break;
                default: break;
                }
            }

            shader_run_frag(g_shader);

            struct Node *color_node = shader_outvar(g_shader, g_shader->scope_frag, "gr_color");
            struct Node **rgb = color_node->vardef_value->vec_values;

            uint32_t hex = 0x00000000 |
                (int)rgb[0]->float_value << 16 |
                (int)rgb[1]->float_value << 8 |
                (int)rgb[2]->float_value;

            int idx = y * g_w + i;

            if (idx >= 0 && idx < g_w * g_h)
            {
                if (z < g_zbuf[idx])
                {
                    g_scr[idx] = hex;
                    g_zbuf[idx] = z;
                }
            }
        }

        l1->x += 1.f / l1->slopex;
        l2->x += 1.f / l2->slopex;

        l1->z += l1->slopez;
        l2->z += l2->slopez;
    }
}

