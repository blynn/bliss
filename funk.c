#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "gen.h"

struct ftree_s {
    double (* eval)(struct ftree_s *, double *);
    int i;
    double d;
    darray_t child;
};

typedef struct ftree_s ftree_t[1];
typedef struct ftree_s *ftree_ptr;

void ftree_init(ftree_ptr f)
{
    darray_init(f->child);
}

ftree_ptr ftree_new()
{
    ftree_ptr res;
    res = malloc(sizeof(ftree_t));
    ftree_init(res);
    return res;
}

void ftree_clear(ftree_ptr f)
{
    int i;

    for (i=0; i<f->child->count; i++) {
	ftree_clear(f->child->item[i]);
	free(f->child->item[i]);
    }
    darray_clear(f->child);
}

double ftree_eval(ftree_ptr f, double *port)
{
    return f->eval(f, port);
}

double ft_mul(ftree_ptr f, double *port)
{
    double res;
    ftree_ptr f0, f1;

    f0 = f->child->item[0];
    f1 = f->child->item[1];
    res = ftree_eval(f0, port) * ftree_eval(f1, port);
    return res;
}

double ft_constant(ftree_ptr f, double *port)
{
    return f->d;
}

double ft_port(ftree_ptr f, double *port)
{
    return port[f->i];
}

struct funk_data_s {
    ftree_ptr root;
    char *program;
};

typedef struct funk_data_s funk_data_t[1];
typedef struct funk_data_s *funk_data_ptr;

enum {
    t_lparen,
    t_rparen,
    t_comma,
    t_constant,
    t_id,
    t_end,
};

struct {
    int type;
    char s[128];
    double d;
} token;

char *program;
int program_i;

void get_number()
{
    double d = 0.0;
    int afterdot = 0;
    double frac = 0.1;
    token.type = t_constant;

    for(;;) {
	if (afterdot) {
	    d += frac * (program[program_i] - '0');
	    frac *= 0.1;
	} else {
	    d *= 10.0;
	    d += program[program_i] - '0';
	}
	program_i++;
	if (program[program_i] == '.') {
	    afterdot = 1;
	    program_i++;
	}
	if (!isdigit(program[program_i])) break;
    }

    token.d = d;
}

void get_id()
{
    int i = 0;
    for (;;) {
	token.s[i] = program[program_i];
	program_i++;
	i++;
	if (!isalnum(program[program_i])) break;
    }
    token.s[i] = 0;
    token.type = t_id;
}

void get_token()
{
    while (isspace(program[program_i])) {
	program_i++;
    }

    if (isdigit(program[program_i])) {
	get_number();
	return;
    }

    if (isalpha(program[program_i])) {
	get_id();
	return;
    }

    switch(program[program_i]) {
	case '(':
	    token.type = t_lparen;
	    program_i++;
	    return;
	case ')':
	    token.type = t_rparen;
	    program_i++;
	    return;
	case ',':
	    token.type = t_comma;
	    program_i++;
	    return;
	case '\0':
	    token.type = t_end;
	    return;
	default:
	    printf("unknown token!\n");
	    break;
    }
}

ftree_ptr parse_expr();

void parse_arg(ftree_ptr f)
{
    get_token();
    if (token.type != t_lparen) {
	printf("expected '('\n");
	return;
    }
    for (;;) {
	ftree_ptr arg = parse_expr();
	darray_append(f->child, arg);
	get_token();
	if (token.type == t_rparen) {
	    break;
	} else if (token.type != t_comma) {
	    printf("parse error!\n");
	}
    }
}

ftree_ptr parse_expr()
{
    ftree_ptr res;
    res = ftree_new();
    get_token();
    switch(token.type) {
	case t_constant:
	    res->eval = ft_constant;
	    res->d = token.d;
	    return res;
	case t_id:
	    if (token.s[0] == 'x') {
		if (isdigit(token.s[1])) {
		    res->i = token.s[1] - '0';
		    res->eval = ft_port;
		    return res;
		} else {
		    printf("parse error!\n");
		    return res;
		}
	    }
	    if (!strcmp(token.s, "mul")) {
		res->eval = ft_mul;
		parse_arg(res);
		return res;
	    }
	default:
	    printf("parse error!\n");
	    return res;
    }
}

ftree_ptr parse(char *s)
{
    program_i = 0;
    program = s;
    return parse_expr();
}

void *funk_note_on()
{
    int *ip = (int *) malloc(sizeof(int));
    *ip = 0;
    return (void *) ip;
}

void funk_note_free(void *data)
{
    free(data);
}

double funk_tick(gen_t g, void *data, double *value)
{
    double res;
    funk_data_ptr p;
    int *ip;

    ip = (int *) data;
    p = (funk_data_ptr) g->data;
    res = ftree_eval(p->root, value);
    *ip = (*ip) + 1;
    return res;
}

void funk_clear_program(gen_ptr g)
{
    funk_data_ptr p;
    p = (funk_data_ptr) g->data;
    free(p->program);
    ftree_clear(p->root);
    free(p->root);
}

static char *strclone(char *s)
{
    char *res = malloc(strlen(s) + 1);
    strcpy(res, s);
    return res;
}

void funk_init_program(gen_ptr g, char *s)
{
    funk_data_ptr p;
    p = (funk_data_ptr) g->data;
    p->program = strclone(s);
    p->root = parse(s);
}

char *funk_get_program(gen_ptr g)
{
    funk_data_ptr p;
    p = (funk_data_ptr) g->data;
    return p->program;
}

void funk_init(gen_ptr g)
{
    g->data = malloc(sizeof(funk_data_t));
    funk_init_program(g, "0.0");
}

void funk_clear(gen_ptr g)
{
    free(g->data);
}

param_ptr funk_param_list[] = { NULL };

gen_info_ptr funk_info_n(int n)
{
    int i;
    gen_info_ptr res = malloc(sizeof(gen_info_t));
    char **port_list;
    static char s[80];

    res->init = funk_init;
    res->id = malloc(5 * sizeof(char));
    strcpy(res->id, "funk0");
    res->id[4] += n;
    res->clear = funk_clear;
    res->note_on = funk_note_on;
    res->note_free = funk_note_free;
    res->tick = funk_tick;
    res->port_count = n;
    port_list = malloc(sizeof(char) * (n+1));
    for(i=0; i<n; i++) {
	sprintf(s, "x%d", i);
	port_list[i] = malloc(sizeof(char) * strlen(s));
	strcpy(port_list[i], s);
    }
    port_list[i] = NULL;
    res->port_name = port_list;
    res->param_count = 0;
    res->param = funk_param_list;
    return res;
}
