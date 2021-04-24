#ifndef GS_BNF_H_INCLUDED
#define GS_BNF_H_INCLUDED
#include <stddef.h>

typedef struct
{
    enum
    {
        TRM_LIT,
        TRM_RULE,
        TRM_TERM
    } type;
    union
    {
        char *literal;
        struct bnf_rule *rule;
    } value;
} bnf_term;

typedef struct
{
    size_t term_number;
    bnf_term *terms;
} bnf_list;

typedef struct bnf_rule
{
    char *name;
    size_t list_number;
    bnf_list *lists;
} bnf_rule;

typedef struct bnf_syntax
{
    size_t rule_number;
    bnf_rule *rules;
} bnf_syntax, bnf_grammar;

bnf_grammar parse_grammar(char[]);
void free_grammar(bnf_grammar);
#endif
