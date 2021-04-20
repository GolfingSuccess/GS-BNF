#include <stdio.h>
#include "bnf-features.h"

int main()
{
    bnf_grammar x = parse_grammar("<abenarthy> ::= <liouville>\n"
                                  "<liouville> ::= <ball> | \"kka'kka\" 'ab\"cy'\n");
    if (!x.rule_number)
    {
        puts("BLYAT");
        return 1;
    }
    print_grammar_repr(x);
    free_grammar(x);
    return 0;
}
