#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "util.h"
#include "base64.h"

char *argsplit(char *buf)
{
    char *arg;

    arg = strchr(buf, ' ');
    if (arg) {
	*arg = 0;
	arg++;
    }
    return arg;
}

void node_init(node_ptr n, char *id)
{
    n->leaf_flag = 0;
    n->id = strclone(id);
    darray_init(n->child);
    n->data = NULL;
    n->len = 0;
}

node_ptr node_new(char *id)
{
    node_ptr n;
    n = (node_ptr) malloc(sizeof(struct node_s));
    node_init(n, id);
    return n;
}

void node_init_leaf(node_ptr n, char *id, char *text)
{
    n->leaf_flag = 1;
    n->id = strclone(id);
    if (text) n->text = strclone(text);
    else n->text = NULL;
    n->data = NULL;
    n->len = 0;
}

node_ptr node_new_leaf(char *id, char *text)
{
    node_ptr n;
    n = (node_ptr) malloc(sizeof(struct node_s));
    node_init_leaf(n, id, text);
    return n;
}

void node_add(node_ptr n, node_ptr child)
{
    if (n->leaf_flag) {
	//BUG!
    }
    darray_append(n->child, child);
}

void node_clear(node_ptr n)
{
    if (n->leaf_flag) free(n->text);
    else darray_clear(n->child);
    if (n->data) free(n->data);
    free(n->id);
}

node_ptr node_list_at(node_ptr n, char *id)
{
    node_ptr n1;
    darray_ptr a = n->child;
    int i;
    for (i=0; i<a->count; i++) {
	n1 = (node_ptr) a->item[i];
	if (!strcmp(n1->id, id)) {
	    return n1;
	}
    }
    return NULL;
}

char *node_text_at(node_ptr n, char *id)
{
    node_ptr n1;
    darray_ptr a = n->child;
    int i;
    for (i=0; i<a->count; i++) {
	n1 = (node_ptr) a->item[i];
	if (!strcmp(n1->id, id)) {
	    if (n1->leaf_flag) {
		return n1->text;
	    }
	}
    }
    return NULL;
}

void tree_print(node_ptr n)
{
    if (n->leaf_flag) {
	printf("leaf %s: %s\n", n->id, n->text);
    } else {
	darray_ptr a = n->child;
	int i;
	printf("block %s:\n", n->id);
	for (i=0; i<a->count; i++) {
	    tree_print((node_ptr) a->item[i]);
	}
	printf("end block %s:\n", n->id);
    }
}

void tree_free(node_ptr n)
{
    node_ptr n1;
    darray_ptr a = n->child;
    int i;

    if (n->leaf_flag) {
	for (i=0; i<a->count; i++) {
	    n1 = (node_ptr) a->item[i];
	    tree_free(n1);
	}
    }

    node_clear(n);
    free(n);
}

static void skip_whitespace(FILE *fp)
{
    char c;
    for (;;) {
	c = fgetc(fp);
	if (!strchr(" \t\n", c)) {
	    ungetc(c, fp);
	    return;
	}
    }
}

void tree_read(node_ptr root, FILE *fp)
{
    //TODO: fix buffer overflows
    char buf[1024];
    char *arg;
    node_ptr nodestack[32];
    int sp;

    //read into tree
    sp = 0;
    nodestack[sp] = root;
    for (;;) {
	skip_whitespace(fp);
	fgets(buf, 1024, fp);
	if (feof(fp)) break;
	buf[strlen(buf) - 1] = 0;
	arg = argsplit(buf);

	if (!strcmp(buf, "begin")) {
	    node_ptr n;
	    if (sp == 32) return;
	    n = node_new(arg);
	    node_add(nodestack[sp], n);
	    sp++;
	    nodestack[sp] = n;
	} else if (!strcmp(buf, "base64")) {
	    node_ptr n = node_new_leaf(arg, NULL);
	    node_add(nodestack[sp], n);
	    base64_decode(&n->data, &n->len, fp);
	} else if (!strcmp(buf, "end")) {
	    sp--;
	    if (sp < 0) return;
	} else {
	    node_ptr n;
	    n = node_new_leaf(buf, arg);
	    node_add(nodestack[sp], n);
	}
    }
}

