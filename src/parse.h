#pragma once

#include "stdlib.h"
#include "stdint.h"

#define PARSE_MAX_DEPTH 100
#define PARSE_MAX_STR 1024

enum val_type {
    VAL_INT,
    VAL_STR,
    VAL_ATOM,
    VAL_IDENT,
    VAL_CONS,
};

struct val;


struct val {
    enum val_type type;
    union {
        intmax_t i;
        char *s;
        struct {
            struct val *car, *cdr;
        };
    };
};

int parse_expr(char *src, struct val **val, char **err);
struct val *val_null(void);
struct val *val_int(intmax_t i);
struct val *val_str(char *s);
struct val *val_atom(char *s);
struct val *val_ident(char *s);
struct val *val_cons(struct val *car, struct val *cdr);
void val_free(struct val *v);
void val_print(struct val *v);
char *val_repr(struct val *v);
