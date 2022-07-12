#include "scope.h"
#include <string.h>


struct Scope *scope_alloc()
{
    struct Scope *s = malloc(sizeof(struct Scope));
    s->layers = 0;
    s->nlayers = 0;

    scope_push_layer(s);

    return s;
}


void scope_free(struct Scope *s)
{
    scope_clear(s);
    free(s->layers);
    free(s);
}


void scope_push_layer(struct Scope *s)
{
    s->layers = realloc(s->layers, sizeof(struct ScopeLayer) * ++s->nlayers);
    s->layers[s->nlayers - 1] = (struct ScopeLayer){ 0, 0, 0, 0 };

    s->top = &s->layers[s->nlayers - 1];
}


void scope_pop_layer(struct Scope *s)
{
    s->layers = realloc(s->layers, sizeof(struct ScopeLayer) * --s->nlayers);
    s->top = &s->layers[s->nlayers - 1];
}


struct Node *scope_find_vardef(struct Scope *s, const char *name, bool error)
{
    for (size_t i = 0; i < s->nlayers; ++i)
    {
        for (size_t j = 0; j < s->layers[i].nvardefs; ++j)
        {
            struct Node *def = s->layers[i].vardefs[j];

            if (strcmp(def->vardef_name, name) == 0)
                return def;
        }
    }

    if (error)
    {
        fprintf(stderr, "[scope_find_vardef] No variable named '%s'.\n", name);
        exit(EXIT_FAILURE);
    }

    return 0;
}


struct Node **scope_get_vardef_type(struct Scope *s, int type, bool copy, size_t *n)
{
    struct Node **defs = 0;
    *n = 0;

    for (size_t i = 0; i < s->nlayers; ++i)
    {
        struct ScopeLayer *layer = &s->layers[i];

        for (size_t j = 0; j < layer->nvardefs; ++j)
        {
            struct Node *def = layer->vardefs[j];

            if (def->vardef_modifier == type)
            {
                defs = realloc(defs, sizeof(struct Node*) * ++*n);
                defs[*n - 1] = copy ? node_copy(def) : def;
            }
        }
    }

    return defs;
}


struct Node *scope_find_fdef(struct Scope *s, const char *name, bool error)
{
    for (size_t i = 0; i < s->nlayers; ++i)
    {
        for (size_t j = 0; j < s->layers[i].nfdefs; ++j)
        {
            struct Node *fdef = s->layers[i].fdefs[j];

            if (strcmp(fdef->fdef_name, name) == 0)
                return fdef;
        }
    }

    if (error)
    {
        fprintf(stderr, "[scope_find_fdef] No function named '%s'.\n", name);
        exit(EXIT_FAILURE);
    }

    return 0;
}


void scope_add_vardef(struct Scope *s, struct Node *n)
{
    s->top->vardefs = realloc(s->top->vardefs, sizeof(struct Node*) * ++s->top->nvardefs);
    s->top->vardefs[s->top->nvardefs - 1] = n;
}


void scope_add_fdef(struct Scope *s, struct Node *n)
{
    s->top->fdefs = realloc(s->top->fdefs, sizeof(struct Node*) * ++s->top->nfdefs);
    s->top->fdefs[s->top->nfdefs - 1] = n;
}


void scope_clear(struct Scope *s)
{
    for (size_t i = 0; i < s->nlayers; ++i)
    {
        struct ScopeLayer *layer = &s->layers[i];

        for (size_t j = 0; j < layer->nvardefs; ++j)
            node_free(layer->vardefs[j]);

        free(layer->vardefs);

        for (size_t j = 0; j < layer->nfdefs; ++j)
            node_free(layer->fdefs[j]);

        free(layer->fdefs);
    }

    free(s->layers);

    s->layers = 0;
    s->nlayers = 0;
    s->top = 0;

    scope_push_layer(s);
}

