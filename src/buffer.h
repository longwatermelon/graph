#ifndef LIBGRAPH_BUFFER_H
#define LIBGRAPH_BUFFER_H

#include <sys/types.h>

struct Buffer
{
    float *data;
    size_t data_len;
};

struct Buffer *graph_gen_buffer();
void graph_delete_buffer(struct Buffer *b);

void graph_bind_buffer(struct Buffer *b);
void graph_buffer_data(size_t size, float *data);

struct Buffer *graph_buffer_bound();

#endif

