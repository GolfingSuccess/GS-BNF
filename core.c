#include <stdlib.h>
#include <string.h>
#include "bnf.h"

#define LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define ALNUM LETTERS "0123456789"
#define CHARACTERS ALNUM "| !#$%&()*+,-./:;>=<?@[\\]^_`{}~"
#define BELONGS(set, chr) (!!strchr((set), (chr)))

static int is_opt_whitespace(char x[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
        if (x[i] != ' ')
            return 0;
    return 1;
}

static int is_rule_name(char x[], size_t size)
{
    if (!size || !BELONGS(LETTERS, x[0]))
        return 0;
    for (size_t i = 1; i < size; ++i)
        if (!BELONGS(ALNUM "-", x[i]))
            return 0;
    return 1;
}

static int is_term(char x[], size_t size)
{
    char quote;
    if (size < 2)
        return 0;
    if (x[0] == '<' && x[size - 1] == '>' && is_rule_name(x + 1, size - 2))
        return 1;
    if (x[0] != x[size - 1])
        return 0;
    quote = x[0] == '"' ? '\'' : x[0] == '\'' ? '"'
                                              : '\0';
    if (!quote)
        return 0;
    for (size_t i = 1; i < size - 1; ++i)
        if (x[i] != quote && !BELONGS(CHARACTERS, x[i]))
            return 0;
    return 1;
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

bnf_syntax parse_grammar(char x[])
{
    size_t index = 0, len = strlen(x), tmp, rnumber = 0;
    bnf_rule *rules = NULL;
    bnf_syntax s = {0, NULL};
    if (!len)
        return s;
    while (index != len)
    {
        tmp = findlen(is_rule, x, len, index);
        if (tmp == -1)
            return s;
        index += tmp;
    }
    index = 0;
    while (index != len)
    {
        rules = realloc(rules, ++rnumber * sizeof(bnf_rule));
        index += findlen(is_opt_whitespace, x, len, index) + 1;
        tmp = findlen(is_rule_name, x, len, index);
        rules[rnumber - 1].name = malloc(tmp + 1);
        memcpy(rules[rnumber - 1].name, x + index, tmp);
        rules[rnumber - 1].name[tmp] = '\0';
        index += tmp + 1;
        index += findlen(is_opt_whitespace, x, len, index) + 3;
        index += findlen(is_opt_whitespace, x, len, index);
        index += findlen(is_expression, x, len, index);
        index += findlen(is_line_end, x, len, index);
    }
    index = 0;
    s.rule_number = rnumber;
    s.rules = rules;
    for (size_t i = 0; i < rnumber; ++i)
    {
        index += findlen(is_opt_whitespace, x, len, index) + 1;
        index += findlen(is_rule_name, x, len, index) + 1;
        index += findlen(is_opt_whitespace, x, len, index) + 3;
        index += findlen(is_opt_whitespace, x, len, index);
        rules[i].expr.list_number = 0;
        rules[i].expr.lists = NULL;
        while (1)
        {
            rules[i].expr.lists = realloc(rules[i].expr.lists, ++rules[i].expr.list_number * sizeof(bnf_list));
            rules[i].expr.lists[rules[i].expr.list_number - 1].term_number = 0;
            rules[i].expr.lists[rules[i].expr.list_number - 1].terms = NULL;
            while ((tmp = findlen(is_term, x, len, index += findlen(is_opt_whitespace, x, len, index))) != -1)
            {
                rules[i].expr.lists[rules[i].expr.list_number - 1].terms = realloc(rules[i].expr.lists[rules[i].expr.list_number - 1].terms,
                                                                                   ++rules[i].expr.lists[rules[i].expr.list_number - 1].term_number * sizeof(bnf_term));
                if (x[index] == '<')
                {
                    rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].type = TRM_RULE;
                    rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].value.rule = NULL;
                    for (size_t j = 0; j < rnumber; ++j)
                        if (!strncmp(rules[j].name, x + index + 1, findlen(is_rule_name, x, len, index + 1)))
                        {
                            rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].value.rule = rules + j;
                            break;
                        }
                    if (!rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].value.rule)
                        rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].type = TRM_TERM;
                }
                else
                    rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].type = TRM_LIT;
                if (rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].type != TRM_RULE)
                {
                    memcpy(rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].value.literal =
                               malloc(tmp - 1),
                           x + index + 1,
                           tmp - 2);
                    rules[i].expr.lists[rules[i].expr.list_number - 1].terms[rules[i].expr.lists[rules[i].expr.list_number - 1].term_number - 1].value.literal[tmp - 2] = '\0';
                }
                index += tmp;
            }
            if ((tmp = findlen(is_line_end, x, len, index)) != -1)
                break;
            index += findlen(is_opt_whitespace, x, len, index) + 1;
            index += findlen(is_opt_whitespace, x, len, index);
        }
        index += tmp;
    }
    return s;
}

void free_grammar(bnf_syntax x)
{
    for (size_t rule = 0; rule < x.rule_number; ++rule)
    {
        free(x.rules[rule].name);
        for (size_t list = 0; list < x.rules[rule].expr.list_number; ++list)
        {
            for (size_t term = 0; term < x.rules[rule].expr.lists[list].term_number; ++term)
                if (x.rules[rule].expr.lists[list].terms[term].type != TRM_RULE)
                    free(x.rules[rule].expr.lists[list].terms[term].value.literal);
            free(x.rules[rule].expr.lists[list].terms);
        }
        free(x.rules[rule].expr.lists);
    }
    free(x.rules);
}
