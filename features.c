#include <stdio.h>
#include <string.h>
#include "bnf.h"

void print_grammar_repr(bnf_grammar gr)
{
    for (size_t rule = 0; rule < gr.rule_number; ++rule)
    {
        printf("<%s> ::=", gr.rules[rule].name);
        for (size_t list = 0; list < gr.rules[rule].expr.list_number; ++list)
        {
            if (list)
                fputs(" |", stdout);
            for (size_t term = 0; term < gr.rules[rule].expr.lists[list].term_number; ++term)
            {
                putchar(' ');
                if (gr.rules[rule].expr.lists[list].terms[term].type == TRM_RULE)
                    printf("<%s>", gr.rules[rule].expr.lists[list].terms[term].value.rule->name);
                else
                    printf(gr.rules[rule].expr.lists[list].terms[term].type == TRM_TERM ? "<%s>" : strchr(gr.rules[rule].expr.lists[list].terms[term].value.literal, '"') ? "'%s'"
                                                                                                                                                                          : "\"%s\"",
                           gr.rules[rule].expr.lists[list].terms[term].value.literal);
            }
        }
        putchar('\n');
    }
}