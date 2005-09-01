#include <stdio.h>
#include <string.h>
#include "file.h"
#include "gen.h"
#include "version.h"

typedef void (*write_fn)(FILE *, void *);
typedef void (*read_fn)(FILE *, gen_ptr g, int i);

static write_fn write_param[param_count];
static read_fn read_param[param_count];

static void write_string(FILE *fp, void *data)
{
    fprintf(fp, "\"%s\"", (char *) data);
}

static void write_double(FILE *fp, void *data)
{
    fprintf(fp, "%f", *((double *) data));
}

static void read_double(FILE *fp, gen_ptr g, int i)
{
    double d;
    fscanf(fp, "%lf", &d);
    assign_double(g, i, d);
}

static void read_string(FILE *fp, gen_ptr g, int param)
{
    char s[80];
    int i = 0, c;
    for (;fgetc(fp) != '"';);
    for (;;) {
	c = fgetc(fp);
	if (c == '"') break;
	s[i] = c;
	i++;
    }
    s[i] = 0;
    assign_string(g, param, s);
}

void file_init()
{
    void set_fns(int type, write_fn fw, read_fn fr) {
	write_param[type] = fw;
	read_param[type] = fr;
    }
    set_fns(param_double, write_double, read_double);
    set_fns(param_string, write_string, read_string);
}

//TODO: put this function somewhere
static node_ptr node_with_id(graph_ptr graph, char *id)
{
    int i;
    for (i=0; i<graph->node_list->count; i++) {
	node_ptr node = (node_ptr) graph->node_list->item[i];
	node_data_ptr p = (node_data_ptr) node->data;
	if (!strcmp(p->id, id)) {
	    return node;
	}
    }
    return NULL;
}

static void write_graph(FILE *fp, graph_ptr graph)
{
    void write_track_event(void *data) {
	event_ptr e = data;
	fprintf(fp, "    %d ", e->delta);
	switch(e->type) {
	    case ev_noteon:
		fprintf(fp, "noteon %d %d", e->x1, e->x2);
		break;
	    case ev_noteoff:
		fprintf(fp, "noteoff %d", e->x1);
		break;
	}
	fprintf(fp, "\n");
    }

    void write_node(void *data)
    {
	int j;
	node_ptr node = data;
	node_data_ptr p = (node_data_ptr) node->data;

	switch(p->type) {
	    gen_ptr g;
	    voice_ptr voice;
	    ins_ptr ins;
	    graph_ptr graph1;

	    case node_type_unit:
		g = p->gen;

		fprintf(fp, "    unit %s %s ", p->id, g->info->id);
		fprintf(fp, "%d %d\n", node->x, node->y);
		for (j=0; j<g->info->param_count; j++) {
		    fprintf(fp, "    set %s %s ", p->id, g->info->param[j]->id);
		    write_param[g->info->param[j]->type](fp, g->param[j]);
		    fprintf(fp, "\n");
		}
		break;
	    case node_type_voice:
		voice = p->voice;
		graph1 = voice->graph;
	    
		fprintf(fp, "  voice %s %d %d %d %d {\n", voice->id,
			voice->notemin, voice->notemax,
			node->x, node->y);
		write_graph(fp, graph1);
		fprintf(fp, "  }\n");
		break;
	    case node_type_ins:
		ins = p->ins;
		graph1 = ins->graph;
	    
		fprintf(fp, "ins %s %d %d {\n", ins->id,
			node->x, node->y);
		write_graph(fp, graph1);
		fprintf(fp, "  track {\n");

		darray_forall(ins->track->event, write_track_event);

		fprintf(fp, "  }\n");
		fprintf(fp, "}\n");

		break;
	}
    }

    void write_edge(void *data)
    {
	edge_ptr e = data;
	node_data_ptr p0, p1;
	p0 = (node_data_ptr) e->src->data;
	p1 = (node_data_ptr) e->dst->data;
	fprintf(fp, "    connect %s %s %d\n", p0->id, p1->id, *((int *) e->data));
    }

    darray_forall(graph->node_list, write_node);
    darray_forall(graph->edge_list, write_edge);
}

void file_save(char *filename, orch_ptr orch)
{
    {
	FILE *fp;

	fp = fopen(filename, "wb");

	fprintf(fp, "bliss %s\n", VERSION_STRING);

	write_graph(fp, orch->graph);

	fclose(fp);
    }
}

enum {
    t_voice,
    t_ins,
    t_orch,
};

static int current_type;
static ins_ptr current_ins;
static orch_ptr current_orch;
static voice_ptr current_voice;

static void read_graph(FILE *fp, graph_ptr graph, node_ptr *kludge)
{
    char s[256];
    void read_word() {
	int i;
	int c;
	i = 0;
	for(;;) {
	    c = fgetc(fp);
	    if (c == EOF) {
		s[i] = 0;
		return;
	    }
	    if (!strchr(" \t\r\n", c)) break;
	}
	s[i++] = c;
	for(;;) {
	    c = fgetc(fp);
	    if (c == EOF) {
		s[i] = 0;
		return;
	    }
	    if (strchr(" \t\r\n", c)){
		s[i] = 0;
		return;
	    }
	    s[i++] = c;
	}
    }

    for (;;) {
	read_word();
	if (!strcmp(s, "unit")) {
	    char *id;
	    int x, y;
	    uentry_ptr u;
	    node_ptr v;

	    read_word();
	    id = strclone(s);
	    read_word();
	    u = utable_at(s);
	    if (!u) {
		printf("Unknown unit type!\n");
		break;
	    }
	    read_word();
	    x = atoi(s);
	    read_word();
	    y = atoi(s);

	    if (current_type == t_ins) {
		v = ins_add_unit(current_ins, id, u, x, y);
	    } else { // current_type == t_voice
		v = voice_add_unit(current_voice, id, u, x, y);
	    }
	    if (!strcmp(id, "out")) {
		*kludge = v;
	    }
	    free(id);
	} else if (!strcmp(s, "set")) {
	    node_ptr v;
	    node_data_ptr p;
	    gen_ptr g;
	    int param;

	    read_word();
	    v = node_with_id(graph, s);
	    assert(v);
	    p = v->data;
	    assert(p->type == node_type_unit);
	    g = p->gen;
	    read_word();
	    param = no_of_param(v, s);
	    if (param < 0) {
		printf("No such parameter: %s\n", s);
		read_word();
	    } else {
		read_param[g->info->param[param]->type](fp, g, param);
	    }
	} else if (!strcmp(s, "connect")) {
	    node_ptr v0, v1;
	    int port;
	    read_word();
	    v0 = node_with_id(graph, s);
	    read_word();
	    v1 = node_with_id(graph, s);
	    read_word();
	    port = atoi(s);
	    add_edge(graph, v0, v1, port);
	} else if (!strcmp(s, "voice")) {
	    char *id;
	    int x, y;
	    int min, max;
	    node_ptr v;
	    voice_ptr voice;
	    int save;

	    read_word();
	    id = strclone(s);
	    read_word();
	    min = atoi(s);
	    read_word();
	    max = atoi(s);
	    read_word();
	    x = atoi(s);
	    read_word();
	    y = atoi(s);

	    v = ins_add_voice(current_ins, id, x, y);
	    free(id);
	    voice = node_get_voice(v);
	    voice->notemin = min;
	    voice->notemax = max;

	    save = current_type;
	    current_type = t_voice;
	    current_voice = voice;
	    read_word();
	    //TODO: check s is "{"
	    read_graph(fp, voice->graph, &voice->out);
	    //TODO: check s is "}"
	    current_type = save;
	} else if (!strcmp(s, "ins")) {
	    int x, y;
	    char *id;
	    node_ptr v;
	    ins_ptr ins;
	    int save;

	    read_word();
	    id = strclone(s);
	    read_word();
	    x = atoi(s);
	    read_word();
	    y = atoi(s);

	    v = orch_add_ins(current_orch, id, x, y);
	    free(id);
	    ins = ((node_data_ptr) v->data)->ins;

	    save = current_type;
	    current_type = t_ins;
	    current_ins = ins;

	    read_word();
	    //TODO: check s is "{"
	    read_graph(fp, ins->graph, &ins->out);
	    //TODO: check s is "}"
	    current_type = save;
	} else if (!strcmp(s, "track")) {
	    read_word(); //TODO: check its '{'
	    for(;;) {
		event_ptr e;
		read_word();
		if (!strcmp(s, "}")) break;
		e = malloc(sizeof(event_t));
		e->delta = atoi(s);
		read_word();
		if (!strcmp(s, "noteon")) {
		    e->type = ev_noteon;
		    read_word();
		    e->x1 = atoi(s);
		    read_word();
		    e->x2 = atoi(s);
		} else if (!strcmp(s, "noteoff")) {
		    e->type = ev_noteoff;
		    read_word();
		    e->x1 = atoi(s);
		    e->x2 = 0;
		}
		darray_append(current_ins->track->event, e);
	    }
	} else break;
    }
}

void file_load(char *filename, orch_ptr orch)
{
    //TODO: error handling
    FILE *fp;
    char s[256];

    void read_word() {
	int i;
	int c;
	i = 0;
	for(;;) {
	    c = fgetc(fp);
	    if (c == EOF) {
		s[i] = 0;
		return;
	    }
	    if (!strchr(" \t\r\n", c)) break;
	}
	s[i++] = c;
	for(;;) {
	    c = fgetc(fp);
	    if (c == EOF) {
		s[i] = 0;
		return;
	    }
	    if (strchr(" \t\r\n", c)){
		s[i] = 0;
		return;
	    }
	    s[i++] = c;
	}
    }

    printf("loading...\n");
    fp = fopen(filename, "rb");
    read_word();
    //TODO: check it's "bliss"
    read_word();
    //TODO: check version

    current_type = t_orch;
    current_orch = orch;
    read_graph(fp, orch->graph, NULL);
    fclose(fp);
    printf("done\n");
}
