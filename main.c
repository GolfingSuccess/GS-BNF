#include <stdio.h>
#include "bnf.h"

int main()
{
    bnf_grammar x = parse_grammar("<abenarthy> ::= <liouville>\n"
                                  "<liouville> ::= <ball> | \"kka'kka\" 'ab\"cy'\n");
    if (!x.rule_number)
    {
        puts("BLYAT");
        return 1;
    }
    puts(x.rules[0].expr.lists[0].terms[0].value.rule->expr.lists[1].terms[1].value.literal);
    free_grammar(x);
    return 0;
}
