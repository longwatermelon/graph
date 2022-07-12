#include "attrib.h"
#include <stdlib.h>

struct AttribLayout *g_atl = 0;

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
    for (size_t i = 0; i < g_atl->len; ++i)
        offset += g_atl->layout[i].count;

    g_atl->layout = realloc(g_atl->layout, sizeof(struct AttribLayoutElement) * ++g_atl->len);
    g_atl->layout[g_atl->len - 1] = (struct AttribLayoutElement){ offset, count };
}


void graph_bind_atl(struct AttribLayout *atl)
{
    g_atl = atl;
}


struct AttribLayout *graph_atl_bound()
{
    return g_atl;
}

