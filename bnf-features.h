#include <stdio.h>
#include "bnf.h"

void fprint_grammar_repr(bnf_grammar, FILE *);
void print_grammar_repr(bnf_grammar);
bnf_rule find_rule_by_name(bnf_grammar, char *);
