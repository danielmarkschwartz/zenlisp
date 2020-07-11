#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "parse.h"

// Wrapper around malloc that cannot fail
void *vl_malloc(size_t n) {
    void *ret = malloc(n);
    assert(ret);
    return ret;
}

// Allocates new values that given type
#define VAL_ALLOC(...) {\
    struct val *r = vl_malloc(sizeof *r);\
    *r = (struct val){__VA_ARGS__};\
    return r;\
}
struct val *val_int(intmax_t i)                         VAL_ALLOC(VAL_INT, .i=i)
struct val *val_str(char *s)                            VAL_ALLOC(VAL_STR, .s=strdup(s))
struct val *val_atom(char *s)                           VAL_ALLOC(VAL_ATOM, .s=strdup(s))
struct val *val_ident(char *s)                          VAL_ALLOC(VAL_IDENT, .s=strdup(s))
struct val *val_cons(struct val *car, struct val *cdr)  VAL_ALLOC(VAL_CONS, .car=car, .cdr=cdr)
#undef VAL_ALLOC

// Free()'s value
void val_free(struct val *v) {
    if(!v) return;

    //TODO: convert to non-recursive algorithm
    switch(v->type) {
    case VAL_INT:                                       break;
    case VAL_STR: case VAL_ATOM: case VAL_IDENT:        free(v->s); break;
    case VAL_CONS:                                      val_free(v->car); val_free(v->cdr); break;
    }

    free(v);
}


#define LEFT(n) (PARSE_MAX_STR-n-1)
char *val_repr(struct val *v) {
    if(!v) return strdup("()");

    char buf[PARSE_MAX_STR] = {0}, *car, *cdr;
    size_t n = 0;

    switch(v->type) {
    case VAL_INT:   n += snprintf(&buf[n], LEFT(n), "%ji", v->i); break;
    case VAL_STR:   n += snprintf(&buf[n], LEFT(n), "\"%s\"", v->s); break;
    case VAL_ATOM:  n += snprintf(&buf[n], LEFT(n), ":%s", v->s); break;
    case VAL_IDENT: n += snprintf(&buf[n], LEFT(n), "'%s", v->s); break;
    case VAL_CONS:
        //TODO: convert to non-recursive algorithm
        n += snprintf(&buf[n], LEFT(n), "(%s . %s)",
                car = val_repr(v->car),
                cdr = val_repr(v->cdr));
        free(car); free(cdr);
        break;
    }
    buf[n] = '\0';
    return strdup(buf);
}
#undef LEFT

void val_print(struct val *v) {
    char *s = val_repr(v);
    puts(s);
    free(s);
}

static char *consume_space(char *s) {
    assert(s); while(isspace(*s)) s++; return s;
}

static char *get_untilc(char **s, char c) {
        static char buf[PARSE_MAX_STR];
        size_t n = 0;
        char *t = *s;

        while(*t && *t != c) buf[n++] = *(t++);
        buf[n] = '\0';

        *s = t;
        return buf;
}

static char *get_whilef(char **s, bool(*f)(char)) {
        static char buf[PARSE_MAX_STR];
        size_t n = 0;
        char *t = *s;

        while(*t && f(*t)) buf[n++] = *(t++);
        buf[n] = '\0';

        *s = t;
        return buf;
}


static bool is_oneof(char c, char *s) { return c != 0 && !!strchr(s, c); }

char *ident_init = "_-+!?*&^%$#@<>/=|,";
static bool is_ident_init(char c)   { return isalpha(c) || is_oneof(c, ident_init); }
static bool is_ident(char c)        { return isalnum(c) || is_oneof(c, ident_init); }


#define OPEN_SENTINEL (void*)0x12345679
#define ERR(e) do{*err = e; while(depth) if(stack[--depth] != OPEN_SENTINEL) val_free(stack[depth]); return -(s-src);}while(0)

// Parses a single expr from src string, filling val with the parsed value
//
// Returns the number of characters consumed. Sets val == NULL on EOF (but
// return may be non-zero, white space in string).
//
// On error, a -return is the error offset and err is filled with a
// description string (do not free).


int parse_expr(char *src, struct val **val, char **err) {
    assert(val); assert(err);
    *val = NULL; *err = NULL;

    if(src == NULL) return 0;


    char *s = src;
    struct val *stack[PARSE_MAX_DEPTH] = {0};
    size_t depth = 0;

start:
    do {
        s = consume_space(s);
        if(s[0] == '\0') break;

        if(s[0] == '(')
            { stack[depth++] = OPEN_SENTINEL; s++; goto start;}

        elif(isdigit(*s))
            stack[depth++] = val_int( strtoimax(s, &s, 0) );

        elif(is_ident_init(*s))
            stack[depth++] = val_ident( get_whilef(&s, is_ident) );

        elif(s[0] == '"') { s++;
            stack[depth++] = val_str( get_untilc(&s, '"') );
            if(!*s) ERR("Unmatched \"");
            s++; }

        elif(s[0] == ':')
            { s++; stack[depth++] = val_atom( get_whilef(&s, is_ident) ); }

        elif(s[0] == ')') {
            if(depth == 0) ERR("Unmatched )");
            struct val *v = NULL;
            while(depth > 0 && stack[--depth] != OPEN_SENTINEL)
                v = val_cons(stack[depth], v);
            if(stack[depth] != OPEN_SENTINEL) { val_free(v); ERR("Unmatched )");}
            stack[depth++] = v; s++;
        }
        else ERR("Unexpected character");
    }while(depth > 1);

    if((depth == 1 && stack[0] == OPEN_SENTINEL) || depth > 1) ERR("Unmatched (");
    if(depth == 1) *val = stack[0];

    return s-src;

}
#undef ERR
#undef OPEN_SENTINEL
