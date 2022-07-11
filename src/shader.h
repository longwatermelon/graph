#ifndef SHADER_H
#define SHADER_H

#include "shaderlang/visitor.h"
#include <limits.h>
#include <SDL2/SDL.h>

struct VertFragInfo
{
    vec3 pos;

    struct Node **outputs;
    size_t len;
};

struct Shader
{
    struct Scope *scope_vert, *scope_frag;
    struct Node *root_vert, *root_frag;

    struct Node **inputs;
    size_t ninputs;

    struct Node *main_call;
};

struct VertFragInfo *vfi_alloc(struct Shader *s);
void vfi_free(struct VertFragInfo *vfi);

struct Shader *shader_alloc(const char *vert, const char *frag);
void shader_free(struct Shader *s);

void shader_run_vert(struct Shader *s, SDL_Renderer *rend);
void shader_run_frag(struct Shader *s);
void shader_insert_runtime_inputs(struct Shader *s);
void shader_insert_layout_vars(struct Shader *s, float *start);

void shader_add_input_int(struct Shader *s, const char *name, int i);
void shader_add_input_vec(struct Shader *s, const char *name, float *v, size_t len);
void shader_add_input_float(struct Shader *s, const char *name, float f);
struct Node *shader_new_input(struct Shader *s, const char *name, int type);
void shader_clear_inputs(struct Shader *s);

struct Node *shader_outvar(struct Shader *s, struct Scope *scope, const char *name);

#endif

