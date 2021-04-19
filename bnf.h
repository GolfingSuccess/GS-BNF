#ifndef GS_BNF_H_INCLUDED
#define GS_BNF_H_INCLUDED
enum trm_type
{
    TRM_LIT,
    TRM_RULE
};

union trm_val
{
    char *literal;
    struct bnf_rule *rule;
};

typedef struct
{
    enum trm_type type;
    union trm_val value;
} bnf_term;

typedef struct
{
    size_t term_number;
    bnf_term *terms;
} bnf_list;

typedef struct
{
    size_t list_number;
    bnf_list *lists;
} bnf_expression;

typedef struct bnf_rule
{
    char *name;
    bnf_expression expr;
} bnf_rule;

struct bnf_syntax
{
    size_t rule_number;
    bnf_rule *rules;
};

typedef struct bnf_syntax *bnf_grammar;

bnf_grammar parse_grammar(char []);
void free_grammar(bnf_grammar);
#endif
