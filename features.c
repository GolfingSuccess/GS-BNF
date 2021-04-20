#include <stdio.h>
#include <string.h>
#include "bnf.h"

void fprint_grammar_repr(bnf_grammar gr, FILE *fp)
{
    for (size_t rule = 0; rule < gr.rule_number; ++rule)
    {
        fprintf(fp, "<%s> ::=", gr.rules[rule].name);
        for (size_t list = 0; list < gr.rules[rule].expr.list_number; ++list)
        {
            if (list)
                fputs(" |", fp);
            for (size_t term = 0; term < gr.rules[rule].expr.lists[list].term_number; ++term)
            {
                fputc(' ', fp);
                fprintf(fp,
                        gr.rules[rule].expr.lists[list].terms[term].type == TRM_LIT
                            ? strchr(gr.rules[rule].expr.lists[list].terms[term].value.literal, '"') ? "'%s'" : "\"%s\""
                            : "<%s>",
                        gr.rules[rule].expr.lists[list].terms[term].type == TRM_RULE
                            ? gr.rules[rule].expr.lists[list].terms[term].value.rule->name
                            : gr.rules[rule].expr.lists[list].terms[term].value.literal);
            }
        }
        fputc('\n', fp);
    }
}

int rule_match_test(bnf_rule rule, char *s, size_t size)
{
    // unimplemented
}