#ifndef PARSE_H
#define PARSE_H

#include "darray.h"

struct node_s {
    int leaf_flag;
    char *id;
    char *text;
    darray_t child;
    //for waves and such:
    unsigned char *data;
    int len;
};

typedef struct node_s *node_ptr;
typedef struct node_s node_t[1];

void node_init(node_ptr n, char *id);
node_ptr node_new(char *id);
void node_init_leaf(node_ptr n, char *id, char *text);
node_ptr node_new_leaf(char *id, char *text);
void node_add(node_ptr n, node_ptr child);
void node_clear(node_ptr n);
node_ptr node_list_at(node_ptr n, char *id);
char *node_text_at(node_ptr n, char *id);
void tree_print(node_ptr n);
void tree_free(node_ptr n);
void tree_read(node_ptr root, FILE *fp);
char *argsplit(char *buf);
#endif //PARSE_H
