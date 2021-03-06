#ifndef SHADER_SCOPE_H
#define SHADER_SCOPE_H

#include "node.h"

struct Scope
{
    struct ScopeLayer
    {
        struct Node **vardefs;
        size_t nvardefs;
    } *layers;

    size_t nlayers;

    struct Node **fdefs;
    size_t nfdefs;

    struct ScopeLayer *top;
};

struct Scope *scope_alloc();
void scope_free(struct Scope *s);

void scope_push_layer(struct Scope *s);
void scope_pop_layer(struct Scope *s);

// error: throw error if not found
struct Node *scope_find_vardef(struct Scope *s, const char *name, bool error);
// Out n
// copy: Copy vardefs if true else return original node
struct Node **scope_get_vardef_type(struct Scope *s, VarModifier type, bool copy, size_t *n);
struct Node *scope_find_fdef(struct Scope *s, const char *name, bool error);

void scope_add_vardef(struct Scope *s, struct Node *n);
void scope_add_fdef(struct Scope *s, struct Node *n);

void scope_clear_vardefs(struct Scope *s);

#endif

