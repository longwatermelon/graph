#ifndef LIBGRAPH_RENDER_H
#define LIBGRAPH_RENDER_H

#include <cglm/cglm.h>
#include <SDL2/SDL.h>

typedef struct
{
    float x, slopex, z, slopez;
} RTI;

void graph_init_renderer(int w, int h);
void graph_free_renderer();

void graph_reset_renderer();
void graph_render_result(SDL_Texture *tex);

void graph_draw(SDL_Renderer *rend);

void filled_tri(SDL_Renderer *rend, vec3 points[3]);
void fill_edges(vec3 a, vec3 b, RTI *l1, RTI *l2);

#endif

