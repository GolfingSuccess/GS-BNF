#include <stdio.h>
#include "bnf-features.h"

int main()
{
    bnf_grammar x = parse_grammar("<abenarthy> ::= <liouville> <abenarthy> <liouville>\n"
                                  "<liouville> ::= <ball> | \"kka'kka\" 'ab\"cy'\n");
    print_grammar_repr(x);
    free_grammar(x);
    return 0;
}
