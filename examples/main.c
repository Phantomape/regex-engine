#include <stdio.h>

#include "../src/regex-engine.h"

int main(int argc, char **argv) {
    int i;
    char *post;
    State *start;

    if (argc < 3) {
        fprintf(stderr, "usage: nfa regexp string ...\n");
        return 1;
    }

    post = re2post(argv[1]);
    if (post == NULL) {
        fprintf(stderr, "Invalid regexp %s\n", argv[1]);
        return 1;
    }
}