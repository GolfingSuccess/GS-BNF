#include <stdlib.h>
#include <string.h>
#include "bnf.h"

static int is_letter(char x)
{
    return x == 'A' || x == 'B' || x == 'C' || x == 'D' ||
           x == 'E' || x == 'F' || x == 'G' || x == 'H' ||
           x == 'I' || x == 'J' || x == 'K' || x == 'L' ||
           x == 'M' || x == 'N' || x == 'O' || x == 'P' ||
           x == 'Q' || x == 'R' || x == 'S' || x == 'T' ||
           x == 'U' || x == 'V' || x == 'W' || x == 'X' ||
           x == 'Y' || x == 'Z' || x == 'a' || x == 'b' ||
           x == 'c' || x == 'd' || x == 'e' || x == 'f' ||
           x == 'g' || x == 'h' || x == 'i' || x == 'j' ||
           x == 'k' || x == 'l' || x == 'm' || x == 'n' ||
           x == 'o' || x == 'p' || x == 'q' || x == 'r' ||
           x == 's' || x == 't' || x == 'u' || x == 'v' ||
           x == 'w' || x == 'x' || x == 'y' || x == 'z';
}

static int is_digit(char x)
{
    return x == '0' || x == '1' || x == '2' || x == '3' ||
           x == '4' || x == '5' || x == '6' || x == '7' ||
           x == '8' || x == '9';
}

static int is_symbol(char x)
{
    return x == '|' || x == ' ' || x == '!' || x == '#' ||
           x == '$' || x == '%' || x == '&' || x == '(' ||
           x == ')' || x == '*' || x == '+' || x == ',' ||
           x == '-' || x == '.' || x == '/' || x == ':' ||
           x == ';' || x == '>' || x == '=' || x == '<' ||
           x == '?' || x == '@' || x == '[' || x == '\\' ||
           x == ']' || x == '^' || x == '_' || x == '`' ||
           x == '{' || x == '}' || x == '~';
}

static int is_character(char x)
{
    return is_letter(x) || is_digit(x) || is_symbol(x);
}

static int is_character1(char x)
{
    return is_character(x) || x == '\'';
}

static int is_character2(char x)
{
    return is_character(x) || x == '"';
}

static int is_rule_char(char x)
{
    return is_letter(x) || is_digit(x) || x == '-';
}

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
        if (!is_character1(x[i]))
            return 0;
    return 1;
}

static int is_text2(char x[], size_t size)
{
    for (size_t i = 0; i < size; ++i)
        if (!is_character2(x[i]))
            return 0;
    return 1;
}

static int is_rule_name(char x[], size_t size)
{
    if (!size || !is_letter(x[0]))
        return 0;
    for (size_t i = 1; i < size; ++i)
        if (!is_rule_char(x[i]))
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
