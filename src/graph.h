#ifndef LIBGRAPH_H
#define LIBGRAPH_H

#include "buffer.h"
#include "attrib.h"
#include "render.h"
#include "shader.h"

#define graph_shader_input(shader, type, ...) shader_add_input_##type(shader, __VA_ARGS__)

#endif

