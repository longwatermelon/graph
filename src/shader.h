#ifndef SHADER_H
#define SHADER_H

#include "shaderlang/visitor.h"
#include <limits.h>

struct Shader
{
    struct Scope *scope;
    struct Node *root;

    struct Node **inputs;
    size_t ninputs;
};

struct Shader *shader_alloc(const char *path);
void shader_free(struct Shader *s);

void shader_run(struct Shader *s);
void shader_insert_runtime_inputs(struct Shader *s);

void shader_add_input_int(struct Shader *s, const char *name, int i);
void shader_add_input_vec(struct Shader *s, const char *name, float *v, size_t len);
void shader_add_input_float(struct Shader *s, const char *name, float f);
struct Node *shader_new_input(struct Shader *s, const char *name, int type);
void shader_clear_inputs(struct Shader *s);

struct Node *shader_outvar(struct Shader *s, const char *name);

#endif

