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

    // If we are not at the end of the text string, which is, *text!='\0', and if the 
    // first character of the text string matches the first character of the regular 
    // expression
    if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
        return matchhere(regexp + 1, text + 1);
    return 0;
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