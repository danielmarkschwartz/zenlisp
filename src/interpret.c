#include <assert.h>
#include <stdlib.h>
#include <strings.h>

#include "common.h"
#include "interpret.h"

void ns_push(struct context *ctx) {
    assert(ctx);
    struct ns *ns = zl_malloc(sizeof *ns);
    *ns = (struct ns){0};
    ns->up = ctx->ns;
    ctx->ns = ns;
}

void ns_pop(struct context *ctx) {
    assert(ctx);
    struct ns *up = NULL;
    if(ctx->ns) up = ctx->ns->up;
    ctx->ns = up;
}

void ns_set(struct context *ctx, char *name, struct val *val) {
    assert(ctx); assert(ctx->ns); assert(name); assert(val);

    struct ns *ns = ctx->ns;
    while(ns->n >= ns->c) {
        ns->c = (ns->c > 0)? ns->c*2: NS_INIT_ALLOC;
        ns->names = realloc(ns->names, ns->c * sizeof ns->names[0]);
        ns->vals = realloc(ns->vals, ns->c * sizeof ns->vals[0]);
    }

    ns->names[ns->n] = name;
    ns->vals[ns->n++] = val;
}

struct val *ns_lookup(struct context *ctx, char *name) {
    assert(ctx); assert(name);
    for(struct ns *ns = ctx->ns; ns; ns = ns->up)
        for(int i = 0; i < ns->n; i++)
            if(strcmp(ns->names[i], name) == 0)
                return ns->vals[i];
    return val_err("Ident not defined");
}

static struct val *builtin_plus(struct context *ctx, struct val *args) {
    intmax_t acc = 0;

    for(; args; args = args->cdr) {
        assert(args->type == VAL_CONS);
        struct val *a = args->car;
        if(a->type == VAL_CONS) a = eval(ctx, a);
        if(a->type != VAL_INT) return val_err("Argument not integer");
        acc += a->i;
    }

    return val_int(acc);
}

void ctx_init(struct context *ctx) {
    assert(ctx);
    *ctx = (struct context){0};

    ns_push(ctx);
    ns_set(ctx, "+", val_builtin(builtin_plus));
}

struct val *eval(struct context *ctx, struct val *expr) {
    assert(ctx); assert(expr);

    switch(expr->type) {
    case VAL_IDENT: return ns_lookup(ctx, expr->s);
    case VAL_CONS: break;
    case VAL_ERR: case VAL_INT: case VAL_STR: case VAL_ATOM:
    case VAL_FUNC: case VAL_BUILTIN:
        return expr;
    }

    struct val *ret, *f = eval(ctx, expr->car);
    if(f->type == VAL_FUNC) {
        //TODO: evaluate lisp function
        ret = val_err("Lisp function evaluation not implemented");
    } elif(f->type == VAL_BUILTIN) {
        ret = f->builtin(ctx, expr->cdr);
    } elif(f->type == VAL_ERR) ret = f;
    else ret =  val_err("Could not evaluate car type");

    return ret;
}

