#include <stdio.h>
#include "bnf.h"

void fprint_grammar_repr(bnf_grammar, FILE *);

#define print_grammar_repr(gr) (fprint_grammar_repr((gr), stdout))