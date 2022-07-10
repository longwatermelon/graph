#include "graph.h"
#include "render.h"
#include "buffer.h"
#include "shader/lexer.h"
#include "shader/util.h"
#include "shader/parser.h"
#include "shader/interpreter.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>


int main(int argc, char **argv)
{
    struct Parser *p = parser_alloc("shaders/test.vert");
    struct Node *root = parser_parse(p);

    struct Interpreter *in = interp_alloc();
    interp_visit(in, root);
    interp_run(in);

    node_free(root);
    parser_free(p);
    interp_free(in);
    /* char *contents = util_read_file("shaders/test.vert"); */
    /* struct Lexer *l = lexer_alloc(contents); */

    /* struct Token *t = 0; */

    /* while ((t = lexer_next_token(l))->type != TT_EOF) */
    /* { */
    /*     printf("Type %d value %s\n", t->type, t->value); */
    /*     token_free(t); */
    /* } */

    /* token_free(t); */
    /* lexer_free(l); */

    return 0;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *w = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer *r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    graph_init_renderer(800, 800);

    bool running = true;
    SDL_Event evt;

    SDL_Texture *scr = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 800, 800);

    struct Buffer *b = graph_gen_buffer();
    graph_bind_buffer(b);

    float verts[] = {
        100.f, 100.f, 0.f,
        200.f, 100.f, 0.f,
        200.f, 200.f, 0.f
    };

    graph_buffer_data(sizeof(verts), verts);

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

        graph_draw(r);
        graph_render_result(scr);

        SDL_RenderCopy(r, scr, 0, 0);

        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderPresent(r);
    }

    graph_free_renderer();

    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);

    IMG_Quit();
    SDL_Quit();

    return 0;
}

