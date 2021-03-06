#include <stdlib.h>
#include <string.h>

#include "regex-engine.h"

#define MAX_REGEXP_OBJECTS 30    /* Max number of regex symbols in expression. */
#define MAX_CHAR_CLASS_LEN 40    /* Max length of character-class buffer in.   */

enum { 
    UNUSED, 
    DOT, 
    BEGIN, 
    END, 
    QUESTIONMARK, 
    STAR, 
    PLUS, 
    CHAR, 
    CHAR_CLASS, 
    INV_CHAR_CLASS, 
    DIGIT, 
    NOT_DIGIT, 
    ALPHA, 
    NOT_ALPHA, 
    WHITESPACE, 
    NOT_WHITESPACE,
    BRANCH,
};

static int match_pattern(regex_t* pattern, const char* text);
static int match_charclass(char c, const char* str);
static int match_star(regex_t p, regex_t* pattern, const char* text);
static int match_plus(regex_t p, regex_t* pattern, const char* text);
static int match_one(regex_t p, char c);
static int match_digit(char c);
static int match_alpha(char c);
static int match_alphanum(char c);
static int match_whitespace(char c);
static int match_metachar(char c, const char* str);
static int match_range(char c, const char* str);
int matchhere(char *regexp, char *text);
int match_question(char *regexp, char *text);
int matchstar_shortest(int c, char *regexp, char *text);
static int is_metachar(char c);

static int match_digit(char c) {
    return c >= '0' && c <= '9';
}

static int match_alpha(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static int match_whitespace(char c) {
    // \t: horizontal tab
    // \v: vertical tab
    // \n: ASCII linefeed
    // \f: ASCII formfeed
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\f') || (c == '\v');
}

static int match_alphanum(char c) {
    return match_alpha(c) || match_digit(c);
}

static int match_pattern(regex_t* pattern, const char* text) {
    /*
    do {
        if ((pattern[0].type == UNUSED) || (pattern[1].type == QUESTIONMARK)) {
            return match_question(pattern[0], &pattern[2], text);
        } else if (pattern[1].type == STAR) {
            return match_star(pattern[0], &pattern[2], text);
        } else if (pattern[1].type == PLUS) {
            return match_plus(pattern[0], &pattern[2], text);
        } else if ((pattern[0].type == END) && pattern[1].type == UNUSED) {
            return (text[0] == '\0');
        }
    /*  Branching is not working properly
        else if (pattern[1].type == BRANCH)
        {
        return (matchpattern(pattern, text) || matchpattern(&pattern[2], text));
        }
    } while ((text[0] != '\0') && match_one(*pattern++, *text++));
    */
    return 0;
}

/* re_match: search for regexp anywhere in text */
int re_match(const char *regexp, const char *text) {
    return re_match_pattern(re_compile(regexp), text);
}

/* re_match_pattern: search for pattern anywhere in text and return the index */
int re_match_pattern(re_t pattern, const char *text) {
    if (pattern == 0)
        return -1;

    if (pattern[0].type == BEGIN)
        return (match_pattern(&pattern[1], text) ? 0 : -1);

    int idx = -1;
    do {
        idx += 1;
        if (match_pattern(pattern, text))
            return idx;
    } while (*text++ != '\0');
}

/* re_compile: convert the string of pattern into regexp objects */
re_t re_compile(const char *pattern) {
    static regex_t re_compiled[MAX_REGEXP_OBJECTS];
    static unsigned char ccl_buf[MAX_CHAR_CLASS_LEN];
    int ccl_buf_idx = 1;

    char c;     // current char in pattern
    int i = 0;  // index into pattern
    int j = 0;  // index into re_compiled array

    while (pattern[i] != '\0' && (j + 1 < MAX_REGEXP_OBJECTS)) {
        c = pattern[i];

        switch (c) {
            case '^': {    re_compiled[j].type = BEGIN;           } break;
            case '$': {    re_compiled[j].type = END;             } break;
            case '.': {    re_compiled[j].type = DOT;             } break;
            case '*': {    re_compiled[j].type = STAR;            } break;
            case '+': {    re_compiled[j].type = PLUS;            } break;
            case '?': {    re_compiled[j].type = QUESTIONMARK;    } break;
            case '|': {    re_compiled[j].type = BRANCH;          } break;

            // Escaped character
            case '\\': {
                if (pattern[i + 1] == '\0') {
                    // TODO(X): invalid regexp, throw an error maybe
                    return -1;
                }

                i += 1; // Skip the escape-char
                switch (pattern[i]) {
                    case 'd': {    re_compiled[j].type = DIGIT;            } break;
                    case 'D': {    re_compiled[j].type = NOT_DIGIT;        } break;
                    case 'w': {    re_compiled[j].type = ALPHA;            } break;
                    case 'W': {    re_compiled[j].type = NOT_ALPHA;        } break;
                    case 's': {    re_compiled[j].type = WHITESPACE;       } break;
                    case 'S': {    re_compiled[j].type = NOT_WHITESPACE;   } break;
                    default: {
                        re_compiled[j].type = CHAR;
                        re_compiled[j].ch = pattern[i];
                    } break;
                }
            } break;

            // Character class
            case '[': {
                int buf_begin = ccl_buf_idx;
                if (pattern[i + 1] == '^') {
                    re_compiled[j].type = INV_CHAR_CLASS;
                    i += 1;
                } else {
                    re_compiled[j].type = CHAR_CLASS;
                }

                // Copy character inside [..] into a buffer, the first condition
                // is the normal termination condition while the second condition
                // is only for invalid regexp
                while (pattern[++i] != ']' && pattern[i] != '\0') {
                    if (ccl_buf_idx >= MAX_CHAR_CLASS_LEN) {
                        return -1;
                    }
                    ccl_buf[ccl_buf_idx++] = pattern[i];
                }

                // Catches cases such as [00000000000000000000000000000000000000][
                // I think there is a better way, which is to add lines similar to
                // line 112
                if (ccl_buf_idx >= MAX_CHAR_CLASS_LEN) {
                    return -1;
                }
                ccl_buf[ccl_buf_idx++] = 0;
                re_compiled[j].ccl = &ccl_buf[buf_begin];
            } break;

            default: {
                re_compiled[j].type = CHAR;
                re_compiled[j].ch = c;
            } break;
        }
        i ++;
        j ++;
    }

    // UNUSED is a sentinel used to indicate the end of pattern
    re_compiled[j].type = UNUSED;

    return (re_t) re_compiled;
}

/* match: search for regexp anywhere in text */
int match(char *regexp, char *text) {
    // If the first character of the regular expression is ^ (an anchored match), 
    // any possible match must occur at the beginning of the string.
    if (regexp[0] == '^')
        return matchhere(regexp + 1, text);

    // The regular expression may match anywhere within the string, this could change
    // a little bit cause currently we only return the first matched result if multiple 
    // matches exist
    do {
        // The difference between a do{...}while{...} loop and a while{...} loop is that
        // we don't test the loop termination condition at the beginning of the loop, since 
        // the * operator permits zero-length matches.
        if (matchhere(regexp, text))
            return 1;
    } while (*text++ != '\0');
    return 0;
}

/* matchhere: search for regexp at beginning of text */
int matchhere(char *regexp, char *text) {
    // If the regular expression is at its end (regexp[0] == '\0'), then all previous 
    // tests have succeeded, and thus the regular expression matches the text.
    if (regexp[0] == '\0')  return 1;

    if (regexp[1] == '*')   
        return matchstar_shortest(regexp[0], regexp + 2, text);

    // If the regular expression is a $ at the end of the expression, then the text 
    // matches only if it too is at its end. Generally, the success of a match is 
    // determined by whether the regular expression runs out at the same time as the 
    // text does. If they do run out together, that indicates a match; if one runs out 
    // before the other, there is no match. This is why we have a (regexp[1] == '\0').
    if (regexp[0] == '$' && regexp[1] == '\0')
        return *text == '\0';

    // If the regular expression is a ?, then we would want to match the character before
    // ? 0 or 1 time
    if (regexp[1] == '?')
        return match_question(regexp, text);

    // If we are not at the end of the text string, which is, *text!='\0', and if the 
    // first character of the text string matches the first character of the regular 
    // expression
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
        return matchhere(regexp + 1, text + 1);

    return 0;
}

/* match_question: search for 'c?' in the text */
int match_question(char *regexp, char *text) {
    return ((*text == *regexp || *regexp == '.') && matchhere(regexp + 2, text + 1))
        || matchhere(regexp + 2, text);
}

/* matchstar_shortest: shortest search for 'c*' regexp at beginning of text */
int matchstar_shortest(int c, char *regexp, char *text) {
    do {
        if (matchhere(regexp, text))    return 1;
    } while (*text != '\0' && (*text++ == c || c == '.'));
    return 0;
}

/* matchstar_longest: longest search for 'c*' regexp at beginning of text */
int matchstar_longest(int c, char *regexp, char *text) {
    char *t;
    for (t = text; *t != '\0' && (*t == c || c == '.'); t ++);
    do {
        if (matchhere(regexp, t))   return 1;
    } while (t-- > text);
    return 0;
}

/*
 * ------------------------------------------------------
 * NFA Approach
 * ------------------------------------------------------
 */

State match_state = { Match };

int num_states;

static int listid;

List l1, l2;

/* state: initialize State */
State* state(int c, State *out, State *out1) {
    State *s;

    num_states ++;
    s = malloc(sizeof *s); // Merge with line 42 maybe
    s->lastlist = 0;
    s->c = c;
    s->out = out;
    s->out1 = out1;
    return s;
}

/* frag: initialize Frag */
Frag frag(State *start, Ptrlist *out) {
	Frag n = { start, out };
	return n;
}

/* list1: create singleton list containing just outp. */
Ptrlist* list1(State **outp) {
	Ptrlist *l;
	
	l = (Ptrlist*)outp;
	l->next = NULL;
	return l;
}

/* patch: patch the list of states at out to point to start. */
void patch(Ptrlist *l, State *s) {
	Ptrlist *next;
	
	for(; l; l=next){
		next = l->next;
		l->s = s;
	}
}

/* append: join the two lists l1 and l2, returning the combination. */
Ptrlist* append(Ptrlist *l1, Ptrlist *l2) {
	Ptrlist *oldl1;
	
	oldl1 = l1;
	while(l1->next)
		l1 = l1->next;
	l1->next = l2;
	return oldl1;
}

/* startlist: initialize a list */
List* startlist(State *start, List *l) {
	l->n = 0;
	listid++;
	addstate(l, start);
	return l;
}

/* addstate: add s to l, following unlabeled arrows. */
void addstate(List *l, State *s) {
	if(s == NULL || s->lastlist == listid)
		return;
	s->lastlist = listid;
	if(s->c == Split){
		/* follow unlabeled arrows */
		addstate(l, s->out);
		addstate(l, s->out1);
		return;
	}
	l->s[l->n++] = s;
}

/* ismatch: check whether state list contains a match */
int ismatch(List *l) {
	int i;

	for(i=0; i<l->n; i++)
		if(l->s[i] == &match_state)
			return 1;
	return 0;
}

/* step: step the NFA from the states in clist past the character c,
 * to create next NFA state set nlist.
 */
void step(List *clist, int c, List *nlist) {
	int i;
	State *s;

	listid++;
	nlist->n = 0;
	for(i=0; i<clist->n; i++){
		s = clist->s[i];
		if(s->c == c)
			addstate(nlist, s->out);
	}
}

/* Run NFA to determine whether it matches s. */
int match_nfa(State *start, char *s) {
	int i, c;
	List *clist, *nlist, *t;

	clist = startlist(start, &l1);
	nlist = &l2;
	for(; *s; s++){
		c = *s & 0xFF;
		step(clist, c, nlist);
		t = clist; clist = nlist; nlist = t;	/* swap clist, nlist */
	}
	return ismatch(clist);
}

/* re2post: convert infix regexp to postfix notation. This is kind of tricky to see
 * cause the original algorithm consists of two stages:
 * 1. Insert the operator "." for juxtapostion
 * 2. Convert the regex to RPN
 * 
 * side note: "a*" should be treate as a whole equivalent to "b"
 */
char* re2post(char *re) {
    int num_alternative, num_atom;
    static char buf[8000];
    char *dst;

    // paren array is used to store info every time we encounter "(", so the index of
    // paren array represent the index of "(" and were there an attribute called location,
    // it will be location of "(" in the expression. The use of paren array as a stack is
    // somehow quite similar to context switch.
    struct {
        int num_alternative;
        int num_atom;
    } paren[100], *p; // ???

    p = paren;
    dst = buf;
    num_alternative = 0; // num_alternative indicates how many "|" should be added
    num_atom = 0; // num_atom indicates whether we should add "." into the expression
    if (strlen(re) >= sizeof buf / 2)   return NULL;

	for(; *re; re++){
		switch(*re){
            case '(':
                // The following dot is targeted to expression before the "("
                if(num_atom > 1){
                    --num_atom;
                    *dst++ = '.';
                }
                if(p >= paren+100)
                    return NULL;
                p->num_alternative = num_alternative;
                p->num_atom = num_atom;
                p++;
                num_alternative = 0;
                num_atom = 0;
                break;
            case '|':
                if(num_atom == 0)
                    return NULL;
                while(--num_atom > 0)
                    *dst++ = '.';
                num_alternative++;
                break;
            case ')':
                // ")" should not be the beginning of our expression
                if(p == paren || num_atom == 0)
                    return NULL;
                while(--num_atom > 0) // Still no idea why it is using while instead of if
                    *dst++ = '.';
                for(; num_alternative > 0; num_alternative--)
                    *dst++ = '|';
                --p;
                num_alternative = p->num_alternative;
                num_atom = p->num_atom;
                num_atom++;
                break;
            case '*':
            case '+':
            case '?':
                // These characters should be treated as a whole with the former character,
                // so we simply put them into expression
                if(num_atom == 0)
                    return NULL;
                *dst++ = *re;
                break;
            default:
                // Consider the most common case like "abcd", which will be converted to "ab.c.d."
                if(num_atom > 1){
                    --num_atom;
                    *dst++ = '.';
                }
                *dst++ = *re;
                num_atom++;
                break;
		}
	}
	if(p != paren)
		return NULL;
	while(--num_atom > 0)
		*dst++ = '.';
	for(; num_alternative > 0; num_alternative--)
		*dst++ = '|';
	*dst = 0;
	return buf;
}

/* post2nfa: convert postfix notation to nfa */
State* post2nfa(char *postfix) {
    char *p;
	Frag stack[1000], *stackp, e1, e2, e;
	State *s;
	
	// fprintf(stderr, "postfix: %s\n", postfix);

	if(postfix == NULL)
		return NULL;

	#define push(s) *stackp++ = s
	#define pop() *--stackp

	stackp = stack;
	for(p=postfix; *p; p++){
		switch(*p){
		default:
			s = state(*p, NULL, NULL);
			push(frag(s, list1(&s->out)));
			break;
		case '.':	/* catenate */
			e2 = pop();
			e1 = pop();
			patch(e1.out, e2.start);
			push(frag(e1.start, e2.out));
			break;
		case '|':	/* alternate */
			e2 = pop();
			e1 = pop();
			s = state(Split, e1.start, e2.start);
			push(frag(s, append(e1.out, e2.out)));
			break;
		case '?':	/* zero or one */
			e = pop();
			s = state(Split, e.start, NULL);
			push(frag(s, append(e.out, list1(&s->out1))));
			break;
		case '*':	/* zero or more */
			e = pop();
			s = state(Split, e.start, NULL);
			patch(e.out, s);
			push(frag(s, list1(&s->out1)));
			break;
		case '+':	/* one or more */
			e = pop();
			s = state(Split, e.start, NULL);
			patch(e.out, s);
			push(frag(e.start, list1(&s->out1)));
			break;
		}
	}

	e = pop();
	if(stackp != stack)
		return NULL;

	patch(e.out, &match_state);
	return e.start;
#undef pop
#undef push

}
