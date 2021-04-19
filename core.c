#include <stdlib.h>
#include <string.h>
#include "bnf.h"

#define LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define DIGITS "0123456789"
#define CHARACTERS LETTERS DIGITS "| !#$%&()*+,-./:;>=<?@[\\]^_`{}~"
#define BELONGS(set, chr) (!!strchr((set), (chr)))

static int is_opt_whitespace(char x[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
        if (x[i] != ' ')
            return 0;
    return 1;
}

static int is_text1(char x[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
        if (!BELONGS(CHARACTERS "'", x[i]))
            return 0;
    return 1;
}

static int is_text2(char x[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
        if (!BELONGS(CHARACTERS "\"", x[i]))
            return 0;
    return 1;
}

static int is_rule_name(char x[], size_t size)
{
    if (!size || !BELONGS(LETTERS, x[0]))
        return 0;
    for (size_t i = 1; i < size; ++i)
        if (!BELONGS(LETTERS DIGITS "-", x[i]))
            return 0;
    return 1;
}

static int is_literal(char x[], size_t size)
{
    return size >= 2 && ((x[0] == '"' && x[size - 1] == '"' && is_text1(x + 1, size - 2)) ||
                         (x[0] == '\'' && x[size - 1] == '\'' && is_text2(x + 1, size - 2)));
}

static int is_term(char x[], size_t size)
{
    return is_literal(x, size) || (size >= 2 && x[0] == '<' && x[size - 1] == '>' && is_rule_name(x + 1, size - 2));
}

static size_t findlen(int rule(char[], size_t), char *x, size_t size, size_t offset)
{
    size -= offset;
    x += offset;
    do
        if (rule(x, size))
            return size;
    while (--size != -1);
    return -1;
}

static int is_list(char *x, size_t size)
{
    size_t index = 0, len;
    if (!size)
        return 0;
    do
    {
        len = findlen(is_term, x, size, index);
        if (len == -1)
            return 0;
        index += len;
        if (index == size)
            return 1;
        index += findlen(is_opt_whitespace, x, size, index);
    } while (index != size);
    return 0;
}

static int is_expression(char x[], size_t size)
{
    size_t index = 0, len;
    if (!size)
        return 0;
    do
    {
        len = findlen(is_list, x, size, index);
        if (len == -1)
            return 0;
        index += len;
        if (index == size)
            return 1;
        index += findlen(is_opt_whitespace, x, size, index);
        if (index == size || x[index++] != '|')
            return 0;
        index += findlen(is_opt_whitespace, x, size, index);
    } while (index != size);
    return 0;
}

static int is_line_end(char x[], size_t size)
{
    size_t index = 0;
    if (!size)
        return 0;
    while (index != size)
    {
        index += findlen(is_opt_whitespace, x, size, index);
        if (index == size || x[index++] != '\n')
            return 0;
    }
    return 1;
}

static int is_rule(char x[], size_t size)
{
    size_t index;
    if (!size)
        return 0;
    index = findlen(is_opt_whitespace, x, size, 0);
    if (index == size || x[index++] != '<')
        return 0;
    index += findlen(is_rule_name, x, size, index);
    if (index == -1 || index == size || x[index++] != '>')
        return 0;
    index += findlen(is_opt_whitespace, x, size, index);
    if (index == size || x[index++] != ':' || index == size || x[index++] != ':' || index == size || x[index++] != '=')
        return 0;
    index += findlen(is_opt_whitespace, x, size, index);
    index += findlen(is_expression, x, size, index);
    if (index == -1)
        return 0;
    index += findlen(is_line_end, x, size, index);
    return index == size;
}

static int is_syntax(char x[])
{
    size_t index = 0, len = strlen(x), tmp;
    while (index != len)
    {
        tmp = findlen(is_rule, x, len, index);
        if (tmp == -1)
            return 0;
        index += tmp;
    }
    return 1;
}

static bnf_term parse_term(char x[], size_t xsize, bnf_rule r[], size_t rsize)
{
    char *tmp;
    bnf_term res;
    if (x[0] == '<')
    {
        res.type = TRM_RULE;
        for (size_t i = 0; i < rsize; ++i)
            if (!strncmp(r[i].name, x + 1, xsize - 2))
            {
                res.value.rule = r + i;
                break;
            }
    }
    else
    {
        res.type = TRM_LIT;
        strncpy(tmp = malloc((xsize - 2) + 1), x + 1, xsize - 2);
        res.value.literal = tmp;
    }
    return res;
}

static bnf_list parse_list(char x[], size_t xsize, bnf_rule r[], size_t rsize)
{
    size_t index = 0, len;
    bnf_list res;
    res.term_number = 0;
    res.terms = NULL;
    while (index != xsize)
    {
        res.terms = realloc(res.terms, ++res.term_number * sizeof(bnf_term));
        res.terms[res.term_number - 1] = parse_term(x + index, len = findlen(is_term, x, xsize, index), r, rsize);
        index += len;
        index += findlen(is_opt_whitespace, x, xsize, index);
    }
    return res;
}

static bnf_expression parse_expression(char x[], size_t xsize, bnf_rule r[], size_t rsize)
{
    size_t index = 0, len;
    bnf_expression res;
    res.list_number = 0;
    res.lists = NULL;
    while (1)
    {
        res.lists = realloc(res.lists, ++res.list_number * sizeof(bnf_list));
        res.lists[res.list_number - 1] = parse_list(x + index, len = findlen(is_list, x, xsize, index), r, rsize);
        index += len;
        if (index == xsize)
            return res;
        index += findlen(is_opt_whitespace, x, xsize, index) + 1;
        index += findlen(is_opt_whitespace, x, xsize, index);
    }
    return res;
}

static void define_rule(char x[], size_t xsize, bnf_rule r[], size_t rsize)
{
    size_t index = findlen(is_opt_whitespace, x, xsize, 0) + 1,
           tmp = findlen(is_rule_name, x, xsize, index);
    for (size_t i = 0; i < rsize; ++i)
        if (!strncmp(r[i].name, x + index, tmp))
        {
            index += tmp + 1;
            index += findlen(is_opt_whitespace, x, xsize, index) + 3;
            index += findlen(is_opt_whitespace, x, xsize, index);
            r[i].expr = parse_expression(x + index, findlen(is_expression, x, xsize, index), r, rsize);
            break;
        }
}

static struct bnf_syntax parse_syntax(char x[], size_t xsize, bnf_rule r[], size_t rsize)
{
    size_t index = 0, tmp;
    struct bnf_syntax res = {rsize, r};
    while (index != xsize)
    {
        define_rule(x + index, tmp = findlen(is_rule, x, xsize, index), r, rsize);
        index += tmp;
    }
    return res;
}

bnf_grammar parse_grammar(char x[])
{
    size_t index = 0, len, tmp, rnumber = 0;
    bnf_rule *rules = NULL;
    bnf_grammar s;
    if (!is_syntax(x))
        return NULL;
    len = strlen(x);
    while (index != len)
    {
        rules = realloc(rules, ++rnumber * sizeof(bnf_rule));
        index += findlen(is_opt_whitespace, x, len, index) + 1;
        tmp = findlen(is_rule_name, x, len, index);
        rules[rnumber - 1].name = malloc(tmp + 1);
        strncpy(rules[rnumber - 1].name, x + index, tmp);
        index += tmp + 1;
        index += findlen(is_opt_whitespace, x, len, index) + 3;
        index += findlen(is_opt_whitespace, x, len, index);
        index += findlen(is_expression, x, len, index);
        index += findlen(is_line_end, x, len, index);
    }
    s = malloc(sizeof(struct bnf_syntax));
    *s = parse_syntax(x, strlen(x), rules, rnumber);
    return s;
}

void free_grammar(bnf_grammar x)
{
    for (size_t rule = 0; rule < x->rule_number; ++rule)
    {
        free(x->rules[rule].name);
        for (size_t list = 0; list < x->rules[rule].expr.list_number; ++list)
        {
            for (size_t term = 0; term < x->rules[rule].expr.lists[list].term_number; ++term)
            {
                if (x->rules[rule].expr.lists[list].terms[term].type == TRM_LIT)
                    free(x->rules[rule].expr.lists[list].terms[term].value.literal);
            }
            free(x->rules[rule].expr.lists[list].terms);
        }
        free(x->rules[rule].expr.lists);
    }
    free(x->rules);
    free(x);
}
