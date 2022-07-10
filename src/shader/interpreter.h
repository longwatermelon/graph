#ifndef SHADER_INTERP_H
#define SHADER_INTERP_H

#include "parser.h"
#include "node.h"

struct Interpreter
{
    struct Node **vardefs;
    size_t nvardefs;

    struct Node **fdefs;
    size_t nfdefs;
};

struct Interpreter *interp_alloc();
void interp_free(struct Interpreter *in);

void interp_run(struct Interpreter *in);

struct Node *interp_visit(struct Interpreter *in, struct Node *n);
struct Node *interp_visit_compound(struct Interpreter *in, struct Node *n);

struct Node *interp_visit_vardef(struct Interpreter *in, struct Node *n);
struct Node *interp_visit_var(struct Interpreter *in, struct Node *n);
struct Node *interp_visit_call(struct Interpreter *in, struct Node *n);
struct Node *interp_visit_fdef(struct Interpreter *in, struct Node *n);
struct Node *interp_visit_assignment(struct Interpreter *in, struct Node *n);

struct Node *interp_find_vardef(struct Interpreter *in, const char *name);
struct Node *interp_find_fdef(struct Interpreter *in, const char *name);

#endif

