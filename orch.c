#include "orch.h"
#include "util.h"

void orch_clear(orch_ptr orch)
{
    void del_ins(void *data) {
	node_ptr node = data;
	ins_clear(node_get_ins(node));
    }
    darray_forall(orch->insnode, del_ins);
    darray_clear(orch->insnode);

    graph_forall_node(orch->graph, node_self_clear);
    graph_clear(orch->graph);
}

void orch_init(orch_ptr orch)
{
    darray_init(orch->insnode);
    graph_init(orch->graph);
}

static void orch_del_node(node_ptr node, void *data)
{
    free(node->data);
}

node_ptr orch_add_ins(orch_ptr orch, char *id, int x, int y)
{
    node_ptr node;
    node_data_ptr p;
    ins_ptr ins = ins_new(id);

    p = malloc(sizeof(node_data_t));
    p->type = node_type_ins;
    p->ins = ins;
    p->id = ins->id;

    p->clear_data = NULL;
    p->clear = orch_del_node;

    node = graph_add_node(orch->graph, p);
    darray_append(orch->insnode, node);
    node->x = x;
    node->y = y;

    return node;
}

double orch_tick(orch_ptr orch)
{
    double res = 0.0;
    void ticknode(void *data) {
	node_ptr node = data;
	node_data_ptr p = node->data;
	res += ins_tick(p->ins);
    }

    darray_forall(orch->insnode,  ticknode);
    return res;
}
