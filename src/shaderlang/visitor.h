#ifndef SHADER_VISITOR_H
#define SHADER_VISITOR_H

#include "parser.h"
#include "node.h"
#include "scope.h"

struct Node *visitor_visit(struct Node *n);
struct Node *visitor_visit_compound(struct Node *n);

struct Node *visitor_visit_vardef(struct Node *n);
struct Node *visitor_visit_var(struct Node *n);
struct Node *visitor_visit_call(struct Node *n);
struct Node *visitor_visit_fdef(struct Node *n);
struct Node *visitor_visit_assignment(struct Node *n);
struct Node *visitor_visit_constructor(struct Node *n);
struct Node *visitor_visit_vec(struct Node *n);

// Out n
struct Node **visitor_output_variables(size_t *n);

void visitor_bind_scope(struct Scope *s);
struct Scope *visitor_scope_bound();

#endif

