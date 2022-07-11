#include "attrib.h"
#include <stdlib.h>

struct AttribLayout *g_bound = 0;

struct AttribLayout *graph_gen_atl(size_t stride)
{
    struct AttribLayout *atl = malloc(sizeof(struct AttribLayout));
    atl->stride = stride;

    atl->layout = 0;
    atl->len = 0;

    return atl;
}


void graph_delete_atl(struct AttribLayout *atl)
{
    free(atl->layout);
    free(atl);
}


void graph_atl_add(int count)
{
    int offset = 0;
    for (size_t i = 0; i < g_bound->len; ++i)
        offset += g_bound->layout[i].offset;

    g_bound->layout = realloc(g_bound->layout, sizeof(struct AttribLayoutElement) * ++g_bound->len);
    g_bound->layout[g_bound->len - 1] = (struct AttribLayoutElement){ offset, count };
}


void graph_bind_atl(struct AttribLayout *atl)
{
    g_bound = atl;
}


struct AttribLayout *graph_atl_bound()
{
    return g_bound;
}

