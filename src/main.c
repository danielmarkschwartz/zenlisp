#include <stdio.h>
#include <string.h>

#include "common.h"
#include "parse.h"
#include "interpret.h"

int main(int argc, char **argv) {
    if(argc == 2 && strcmp(argv[1], "-v") == 0) {
        printf("ZenLisp " ZENLISP_VER "\n");
        return 0;
    }

    if(argc > 2) {
        fprintf(stderr, "ERROR: Expected at most 1 argument\n");
        return 1;
    }

    char *buf = NULL;
    size_t bufcap = 0;

    if(argc == 1) {
        if (getdelim(&buf, &bufcap, 0, stdin) <= 0) {
            fprintf(stderr, "ERROR: reading stdin\n");
            return 1;
        }
    } else  buf = argv[1];

    struct context ctx;
    ctx_init(&ctx);

    char *err; struct val *v; int i;
    for(;;) {
        i = parse_expr(buf, &v, &err);
        if(err) { 
            fprintf(stderr, "input: %s\n       ", buf);
            for(int j = 0; j < -i-1; j++) fputc('-', stderr);
            fputc('^', stderr);
            fprintf(stderr, "\nERROR: %s\n", err);
            break;
        }
        if(!v) break;
        struct val *ret = eval(&ctx, v);
        val_print(ret);
        val_free(v);
        val_free(ret);

        buf = &buf[i];
    }

    ctx_free(&ctx);

    return 0;
}
