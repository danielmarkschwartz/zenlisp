#include <stdio.h>
#include "common.h"
#include "parse.h"

int main(int argc, char **argv) {
    printf("ZenLisp " ZENLISP_VER "\n");

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
        val_print(v);
        val_free(v);
        buf = &buf[i];
    }

    return 0;
}
