#include "buffer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct Buffer *g_buffer = 0;

struct Buffer *graph_gen_buffer()
{
    struct Buffer *b = malloc(sizeof(struct Buffer));
    b->data = 0;
    b->data_len = 0;

    return b;
}


void graph_delete_buffer(struct Buffer *b)
{
    if (b->data) free(b->data);
    free(b);
}


void graph_bind_buffer(struct Buffer *b)
{
    g_buffer = b;
}


void graph_buffer_data(size_t size, float *data)
{
    g_buffer->data_len = size / sizeof(float);
    g_buffer->data = malloc(size);
    memcpy(g_buffer->data, data, size);
}


struct Buffer *graph_buffer_bound()
{
    return g_buffer;
}

