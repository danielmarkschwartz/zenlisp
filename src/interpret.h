#pragma once

#include "parse.h"

#define NS_INIT_ALLOC 8

struct ns {
    struct ns *up;
    char **names;
    struct val **vals;
    size_t n, c;
};

struct context {
    struct ns *ns;
};

struct val *eval(struct context *ctx, struct val *expr);
void ns_push(struct context *ctx);
void ns_pop(struct context *ctx);
void ns_set(struct context *ctx, char *name, struct val *val);
struct val *ns_lookup(struct context *ctx, char *name);
void ctx_init(struct context *ctx);
void ctx_free(struct context *ctx);
