#include <stdio.h>

#include "../src/regex-engine.h"

int main(int argc, char **argv) {
    int i;
    char *post;
    State *start;

/*
    if (argc < 3) {
        fprintf(stderr, "usage: nfa regexp string ...\n");
        return 1;
    }
*/
    char *test_re = "a+b";
    post = re2post(test_re);
    if (post == NULL) {
        fprintf(stderr, "Invalid regexp %s\n", argv[1]);
        return 1;
    }
}