#pragma once

#include "stdlib.h"
#include "stdint.h"

#define PARSE_MAX_DEPTH 100
#define PARSE_MAX_STR 1024


enum val_type {
    VAL_ERR,
    VAL_INT,
    VAL_STR,
    VAL_ATOM,
    VAL_IDENT,
    VAL_CONS,
    VAL_FUNC,
    VAL_BUILTIN,
};

struct val;
struct context;

typedef struct val * (*builtin_t)(struct context *, struct val *);

struct val {
    enum val_type type;
    union {
        intmax_t i;
        char *s;
        struct {
            struct val *car, *cdr;
        };
        struct {
            struct val *args;
            struct val *body;
        };
        builtin_t builtin;
    };
};

int parse_expr(char *src, struct val **val, char **err);
struct val *val_err(char *e);
struct val *val_int(intmax_t i);
struct val *val_str(char *s);
struct val *val_atom(char *s);
struct val *val_ident(char *s);
struct val *val_cons(struct val *car, struct val *cdr);
struct val *val_func(struct val *args, struct val *body);
void val_free(struct val *v);
void val_print(struct val *v);
char *val_repr(struct val *v);
struct val *val_builtin(builtin_t b);
