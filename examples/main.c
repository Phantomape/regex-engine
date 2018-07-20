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

    char *test_re = "abc|d";
    post = re2post(test_re);
    if (post == NULL) {
        fprintf(stderr, "Invalid regexp %s\n", argv[1]);
        return 1;
    }

    char *nfa = post2nfa(post);
    if (nfa == NULL) {
        fprintf(stderr, "Error in post2nfa %s\n", post);
        return 1;
    }

    extern int num_states;
    extern List l1, l2;
    l1.s = malloc(num_states * sizeof l1.s[0]);
    l2.s = malloc(num_states * sizeof l2.s[0]);

    char *test_str = "abc";
    if (match_nfa(nfa, test_str))
        printf("%s\n", test_str);

    return 0;
}
