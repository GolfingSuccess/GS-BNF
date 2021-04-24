#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bnf.h"

void fprint_grammar_repr(bnf_grammar gr, FILE *fp)
{
    size_t rule, list, term;
    if (!gr.rule_number)
    {
        fputs("[[[ ILLEGAL BNF SPECIFICATION! ]]]", fp);
        return;
    }
    for (rule = 0; rule < gr.rule_number; ++rule)
    {
        bnf_rule r = gr.rules[rule];
        fprintf(fp, "<%s> ::=", r.name);
        for (list = 0; list < r.list_number; ++list)
        {
            bnf_list l = r.lists[list];
            if (list)
                fputs(" |", fp);
            for (term = 0; term < l.term_number; ++term)
            {
                bnf_term t = l.terms[term];
                fputc(' ', fp);
                fprintf(fp,
                        t.type == TRM_LIT
                            ? strchr(t.value.literal, '"') ? "'%s'" : "\"%s\""
                            : "<%s>",
                        t.type == TRM_RULE ? t.value.rule->name : t.value.literal);
            }
        }
        fputc('\n', fp);
    }
}

void print_grammar_repr(bnf_grammar gr)
{
    fprint_grammar_repr(gr, stdout);
}

bnf_rule find_rule_by_name(bnf_grammar gr, char *name)
{
    size_t rule;
    bnf_rule not_found = {0, NULL, name}, r;
    for(rule = 0; rule < gr.rule_number; ++rule)
    {
        r = gr.rules[rule];
        if (!strcmp(name, r.name))
            return r;
    }
    return not_found;
}
