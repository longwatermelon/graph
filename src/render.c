#include "render.h"
#include "buffer.h"
#include "shader.h"

#define swapv(v1, v2) { \
    vec3 tmp; \
    glm_vec3_copy(v1, tmp); \
    glm_vec3_copy(v2, v1); \
    glm_vec3_copy(tmp, v2); \
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

    struct Buffer *b = graph_buffer_bound();

    for (size_t i = 0; i < b->data_len; i += 9)
    {
        vec3 points[3];
        memcpy(points[0], b->data + i, 3 * sizeof(float));
        memcpy(points[1], b->data + i + 3, 3 * sizeof(float));
        memcpy(points[2], b->data + i + 6, 3 * sizeof(float));

        filled_tri(rend, points);
    }
}

void graph_use_shader(struct Shader *s)
{
    g_shader = s;
}

void filled_tri(SDL_Renderer *rend, vec3 points[3])
{
    vec3 a, b, c;
    glm_vec3_copy(points[0], a);
    glm_vec3_copy(points[1], b);
    glm_vec3_copy(points[2], c);

    if (a[1] > b[1]) swapv(a, b);
    if (a[1] > c[1]) swapv(a, c);
    if (b[1] > c[1]) swapv(b, c);

    SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);

    RTI r_ac = { a[0], (c[1] - a[1]) / (c[0] - a[0]), a[2], (c[2] - a[2]) / (c[1] - a[1]) };
    RTI r_ab = { a[0], (b[1] - a[1]) / (b[0] - a[0]), a[2], (b[2] - a[2]) / (b[1] - a[1]) };
    RTI r_bc = { b[0], (c[1] - b[1]) / (c[0] - b[0]), b[2], (c[2] - b[2]) / (c[1] - b[1]) };

    fill_edges(a, b, &r_ac, &r_ab);
    fill_edges(b, c, &r_ac, &r_bc);
}


void fill_edges(vec3 a, vec3 b, RTI *l1, RTI *l2)
{
    uint32_t hex = 0xFFFFFFFF;

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
            shader_add_input_vec(g_shader, "i_pos", pos, 3);
            shader_add_input_float(g_shader, "i_x", pos[0] / 2.f);
            shader_add_input_float(g_shader, "i_y", pos[1] / 2.f);
            shader_run(g_shader);

            struct Node *color_node = shader_outvar(g_shader, "gr_color");
            hex = 0x000000 | (int)color_node->vardef_value->vec_values[0]->float_value << 16 | (int)color_node->vardef_value->vec_values[1]->float_value << 8 | (int)color_node->vardef_value->vec_values[2]->float_value;
            /* printf("%d\n", (int)color_node->vardef_value->vec3_value[0]); */

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

