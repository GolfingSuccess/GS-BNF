#include <stdio.h>
#include "bnf.h"

int main()
{
    bnf_grammar x = parse_grammar("<abenarthy> ::= <liouville>\n"
                                  "<liouville> ::= <ball> | \"kka'kka\" 'ab\"cy'\n");
    if (!x)
    {
        puts("BLYAT");
        return 1;
    }
    if (!x->rule_number)
    {
        puts("CYKA");
        return 1;
    }
    printf("%s\n", x->rules[0].expr.lists[0].terms[0].value.rule->expr.lists[1].terms[1].value.literal);
    free_grammar(x);
    return 0;
}
