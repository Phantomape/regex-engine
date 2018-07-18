#ifdef __cplusplus
extern "C" {
#endif

typedef struct regex_t {
    unsigned char  type;     /* Enum type: CHAR, STAR, etc.           */
    union {
        unsigned char  ch;   /*      the character itself             */
        unsigned char* ccl;  /*  OR  a pointer to characters in class */
    };
} regex_t;

typedef struct regex_t* re_t;

/* Compile regex string pattern to a regex_t-array. */
regex_t* re_compile(const char *pattern);

/* Find matches of the compiled pattern inside text. */
int re_matchp(regex_t *pattern, const char *text);

/* Find matches of the txt pattern inside text (will compile automatically first). */
int re_match(const char* pattern, const char* text);

/* NFA state data structure */
typedef struct State State;
struct State {
	int c;
	State *out;
	State *out1;
	int lastlist;
};

#ifdef __cplusplus
}
#endif