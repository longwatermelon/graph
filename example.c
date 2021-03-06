#include "graph.h"
#include "util.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *w = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 400, 400, SDL_WINDOW_SHOWN);
    SDL_Renderer *r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    graph_init_renderer(400, 400);

    bool running = true;
    SDL_Event evt;

    SDL_Texture *scr = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 400, 400);

    struct Buffer *b = graph_gen_buffer();
    graph_bind_buffer(b);

    float verts[] = {
        200.f, 150.f, 0.f,  255.f, 0.f, 0.f,
        125.f, 275.f, 0.f,  0.f, 0.f, 255.f,
        275.f, 275.f, 0.f,  0.f, 255.f, 0.f
    };

    graph_buffer_data(sizeof(verts), verts);

    struct AttribLayout *atl = graph_gen_atl(6);
    graph_bind_atl(atl);
    graph_atl_add(3);
    graph_atl_add(3);

    struct Shader *shader = shader_alloc("shaders/vert.grsl", "shaders/frag.grsl");
    graph_use_shader(shader);

    while (running)
    {
        while (SDL_PollEvent(&evt))
        {
            switch (evt.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            }
        }

        SDL_RenderClear(r);

        graph_shader_input(shader, float, "i_time", (float)SDL_GetTicks() / 100.f);
        graph_draw(r);
        graph_render_result(scr);

        SDL_RenderCopy(r, scr, 0, 0);

        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderPresent(r);
    }

    SDL_DestroyTexture(scr);

    graph_delete_buffer(b);
    graph_delete_atl(atl);

    graph_free_renderer();
    shader_free(shader);

    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);

    IMG_Quit();
    SDL_Quit();

    return 0;
}

