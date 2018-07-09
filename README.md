# regex-engine
A toy regex engine following the tutorial https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html

# resources
Found a tutorial very useful [here](https://swtch.com/~rsc/regexp/) and a useful extended code [here](https://github.com/kokke/tiny-regex-c)

# explanation
The function match(regexp, text) tests whether there is an occurrence of the regular expression anywhere within the text; it returns 1 if a match is found and 0 if not. If there is more than one match, it finds the leftmost and shortest.

# future plans
*   Extended code for more syntax support
*   Implemented matching using finite automata