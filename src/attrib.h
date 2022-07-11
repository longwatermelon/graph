#ifndef LIBGRAPH_ATTRIB_H
#define LIBGRAPH_ATTRIB_H

#include <sys/types.h>

struct AttribLayout
{
    struct AttribLayoutElement
    {
        int offset, count;
    } *layout;

    size_t len;
    size_t stride;
};

struct AttribLayout *graph_gen_atl(size_t stride);
void graph_delete_atl(struct AttribLayout *atl);

void graph_atl_add(int count);

void graph_bind_atl(struct AttribLayout *atl);
struct AttribLayout *graph_atl_bound();

#endif

