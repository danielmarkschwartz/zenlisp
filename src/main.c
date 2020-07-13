#include <stdio.h>
#include <string.h>

#include "common.h"
#include "parse.h"
#include "interpret.h"
#include "elf.h"

int main(int argc, char **argv) {
    struct elf elf;
    elf_init(&elf);

    char *text = "this is my text";
    elf_append_text(&elf, text, strlen(text));

    char *data = "this is my data";
    elf_append_data(&elf, data, strlen(data));

    elf.bss_size = 0x1234;

    char *err_str = elf_write(&elf, "test.elf");
    if(err_str) fprintf(stderr, "ERROR: %s\n", err_str);

    return 0;

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

        buf = &buf[i];
    }

    return 0;
}
