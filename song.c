#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "song.h"
#include "pattern.h"
#include "mlist.h"
#include "util.h"
#include "base64.h"
#include "parse.h"

static void calcsamptick(song_ptr s)
{
    if (s->bpm && s->tpb) s->samptick = 60 * samprate / (s->bpm * s->tpb);
}

void song_put_bpm(song_ptr s, int bpm)
{
    s->bpm = bpm;
    calcsamptick(s);
}

void song_put_tpb(song_ptr s, int tpb)
{
    s->tpb = tpb;
    calcsamptick(s);
}

void song_put_bpm_tpb(song_ptr s, int bpm, int tpb)
{
    s->bpm = bpm;
    s->tpb = tpb;
    calcsamptick(s);
}

void song_clear(song_ptr s)
{
    int i, n;
    n = s->machine->count;
    for (i=0; i<n; i++) machine_clear((machine_ptr) s->machine->item[i]);
    darray_clear(s->machine);
    n = s->edge->count;
    for (i=0; i<n; i++) edge_clear((edge_ptr) s->edge->item[i]);
    darray_clear(s->edge);
    for (i=0; i<wave_max; i++) {
	if (s->wave[i]) {
	    free(s->wave[i]);
	}
    }

    darray_clear(s->mid_list);
}

machine_ptr song_machine_at(song_ptr s, char *id)
{
    int i, n;
    darray_ptr a;
    machine_ptr m;

    a = s->machine;
    n = a->count;
    for (i=0; i<n; i++) {
	m = (machine_ptr) a->item[i];
	if (!strcmp(m->id, id)) {
	    return m;
	}
    }
    return NULL;
}

void song_put_wave(song_ptr s, wave_ptr w, int i)
{
    s->wave[i] = w;
}

machine_ptr song_create_machine_auto_id(song_ptr s, char *gearid)
{
    machine_ptr m;
    char *id;
    char temp[10];
    int count;
    machine_info_ptr mi;

    mi = machine_info_at(gearid);
    if (!mi) return NULL;

    //work out unique name
    id = (char *) alloca(strlen(mi->name) + 10);
    strcpy(id, mi->name);
    count = 2;
    for (;;) {
	machine_ptr m;
	m = song_machine_at(s, id);
	if (!m) break;
	strcpy(id, mi->name);
	sprintf(temp, "%d", count);
	strcat(id, temp);
	count++;
    }

    m = machine_new(mi, s, id);
    darray_append(s->machine, m);

    darray_append(s->mid_list, m->id);
    return m;
}

machine_ptr song_create_machine(song_ptr s, char *gearid, char *id)
{
    machine_ptr m;
    machine_info_ptr mi;

    mi = machine_info_at(gearid);
    if (!mi) return NULL;

    m = machine_new(mi, s, id);
    darray_append(s->machine, m);

    darray_append(s->mid_list, m->id);
    return m;
}

void song_del_machine(song_ptr s, machine_ptr m)
{
    int i, n;
    darray_remove(s->machine, m);
    darray_remove(s->mid_list, m->id);
    n = m->in->count;
    for (i=0; i<n; i++) {
	edge_ptr e = m->in->item[i];
	darray_remove(e->src->out, e);
	darray_remove(s->edge, e);
	edge_clear(e);
    }
    n = m->out->count;
    for (i=0; i<n; i++) {
	edge_ptr e = m->out->item[i];
	darray_remove(e->dst->in, e);
	darray_remove(s->edge, e);
	edge_clear(e);
    }
    root_pattern_check(m);
    machine_clear(m);
    free(m);
}

edge_ptr song_create_edge(song_ptr s, machine_ptr src, machine_ptr dst)
{
    edge_ptr e;
    e = edge_new(src, dst);
    darray_append(e->dst->in, e);
    darray_append(e->src->out, e);
    darray_append(s->edge, e);
    return e;
}

void song_del_edge(song_ptr s, edge_ptr e)
{
    darray_remove(e->src->out, e);
    darray_remove(e->dst->in, e);
    darray_remove(s->edge, e);
    free(e);
}

int song_is_connected(song_ptr s, machine_ptr src, machine_ptr dst)
{
    int i, n;
    n = src->out->count;
    for (i=0; i<n; i++) {
	if (((edge_ptr) src->out->item[i])->dst == dst) return 1;
    }
    return 0;
}

void song_next_sample(song_ptr s, double *l, double *r)
{
    if (!s->sampmod) {
	track_ptr t;
	darray_ptr a;
	int i, n;

	a = s->machine;
	n = a->count;
	for (i=0; i<n; i++) {
	    t = ((machine_ptr) a->item[i])->track;
	    track_tick(t, s->tickcount);
	}
    }
    machine_next_sample(s->master, l, r);

    if (!s->sampmod) {
	s->tickcount++;
	s->tickmod++;
	s->tickmod %= s->tpb;
	if (s->tickcount == s->song_end) {
	    song_rewind(s);
	    return;
	}
    }
    s->sampcount++;
    s->sampmod++;
    s->sampmod %= s->samptick;
}

void song_rewind(song_ptr s)
{
    darray_ptr a;
    int i, n;
    machine_ptr m;
    s->sampcount = 0;
    s->sampmod = 0;
    s->tickmod = 0;
    s->tickcount = 0;
    a = s->machine;
    n = a->count;
    for (i=0; i<n; i++) {
	m = (machine_ptr) a->item[i];
	track_rewind(m->track);
    }
}

void song_jump_to_tick(song_ptr s, int tick)
{
    s->tickmod = 0;
    s->tickcount = tick;
}

void song_init(song_ptr s)
{
    int i;

    darray_init(s->machine);
    darray_init(s->edge);
    for (i=0; i<wave_max; i++) s->wave[i] = NULL;
    s->song_end = 16;
    s->loop_end = 16;
    s->loop_begin = 0;

    darray_init(s->mid_list);
}

// TODO: move to separate file
// Song I/O functions

void track_print(track_ptr t, FILE *fp)
{
    tcell_ptr c;
    for (c = t->first->next; c; c = c->next) {
	fprintf(fp, "\t\tcell %d %s\n", c->tick, c->text);
    }
}

void machine_print(machine_ptr m, FILE *fp)
{
    int i, n;
    darray_ptr a;

    a = m->pattern;
    n = a->count;
    fprintf(fp, "begin machine\n");
    fprintf(fp, "\tid %s\n", m->id);
    fprintf(fp, "\tx %d\n", m->x);
    fprintf(fp, "\ty %d\n", m->y);
    fprintf(fp, "\tplugin %s\n", m->mi->id);
    fprintf(fp, "\tbegin init_state\n");
    machine_print_state(m, fp);
    fprintf(fp, "\tend\n");
    fprintf(fp, "\tbegin sequence\n");
    track_print(m->track, fp);
    fprintf(fp, "\tend\n");
    for (i=0; i<n; i++) {
	pattern_ptr p;
	fprintf(fp, "\tbegin pattern\n");
	p = (pattern_ptr) a->item[i];
	pattern_print(p, fp);
	fprintf(fp, "\tend\n");
    }
    fprintf(fp, "end\n");
}

void song_print(song_ptr s, FILE *fp)
{
    int i, n;
    darray_ptr a;

    fprintf(fp, "song_end %d\n", s->song_end);
    fprintf(fp, "loop_begin %d\n", s->loop_begin);
    fprintf(fp, "loop_end %d\n", s->loop_end);
    fprintf(fp, "x %d\n", s->x);
    fprintf(fp, "y %d\n", s->y);
    fprintf(fp, "master %s\n", s->master->id);
    a = s->machine;
    n = a->count;
    for (i=0; i<n; i++) {
	machine_ptr m = (machine_ptr) a->item[i];
	machine_print(m, fp);
    }
    a = s->edge;
    n = a->count;
    for (i=0; i<n; i++) {
	edge_ptr e = (edge_ptr) a->item[i];
	fprintf(fp, "begin connection\n");
	fprintf(fp, "\tfrom %s\n", e->src->id);
	fprintf(fp, "\tto %s\n", e->dst->id);
	fprintf(fp, "end\n");
    }
    for (i=0; i<wave_max; i++) {
	wave_ptr w = s->wave[i];
	if (w) {
	    fprintf(fp, "begin wave\n");
	    fprintf(fp, "\tindex %d\n", i);
	    fprintf(fp, "\tvolume %f\n", w->volume);
	    fprintf(fp, "\tsample_count %d\n", w->sample_count);
	    fprintf(fp, "\troot_note %d\n", w->root_note);
	    //fprintf(fp, "\tdata_length %d\n", w->data_length);
	    fprintf(fp, "end\n");
	}
    }
    for (i=0; i<wave_max; i++) {
	wave_ptr w = s->wave[i];
	if (w) {
	    fprintf(fp, "begin wavedata\n");
	    fprintf(fp, "\tindex %d\n", i);
	    fprintf(fp, "\tbase64 data\n");
	    base64_encode(fp, w->data, w->data_length);
	    fprintf(fp, "end\n");
	}
    }
}

int song_save(song_ptr s, char *filename)
{
    FILE *fp;

    fp = fopen(filename, "w");
    if (!fp) return 1;
    song_print(s, fp);
    fclose(fp);
    return 0;
}

static void parse_pattern(pattern_ptr p, node_ptr root)
{
    int i;
    darray_ptr a = root->child;
    char *id = node_text_at(root, "id");

    p->id = strclone(id);

    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (n->leaf_flag && !strcmp(n->id, "cell")) {
	    char *s1 = strclone(n->text);
	    char *s2, *text;
	    s2 = argsplit(s1);
	    text = argsplit(s2);
	    pattern_put(p, text, atoi(s1), atoi(s2));
	    free(s1);
	}
    }
}

static void parse_track(track_ptr t, node_ptr root)
{
    int i;
    darray_ptr a = root->child;

    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (n->leaf_flag && !strcmp(n->id, "cell")) {
	    char *arg;
	    char *s = strclone(n->text);
	    arg = argsplit(s);
	    track_put(t, arg, atoi(s));
	    free(s);
	}
    }
}

static void parse_machine(song_ptr s, node_ptr root)
{
    int x, y;
    char *id = node_text_at(root, "id");
    char *plugin = node_text_at(root, "plugin");
    machine_ptr m;
    int i;
    darray_ptr a = root->child;

    x = atoi(node_text_at(root, "x"));
    y = atoi(node_text_at(root, "y"));
    m = song_create_machine(s, plugin, id);
    m->x = x;
    m->y = y;

    //create track
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "sequence")) {
	    parse_track(m->track, n);
	    break;
	}
    }

    //create patterns
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "pattern")) {
	    pattern_ptr p = pattern_new(m);
	    parse_pattern(p, n);
	}
    }

    //initialize machine state
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "init_state")) {
	    int j;
	    for (j=0; j<n->child->count; j++) {
		node_ptr n1 = n->child->item[j];
		cell_t c;
		machine_cell_init(c, m, n1->id, j);
		machine_parse(m, c, j);
		cell_clear(c);
	    }
	}
    }
}

static void parse_connection(song_ptr s, node_ptr n)
{
    char *s1 = node_text_at(n, "from");
    char *s2 = node_text_at(n, "to");
    machine_ptr src, dst;

    src = song_machine_at(s, s1);
    dst = song_machine_at(s, s2);
    if (src && dst) {
	song_create_edge(s, src, dst);
    }
}

static void parse_wave(song_ptr s, node_ptr n)
{
    wave_ptr w = wave_new();
    int index;

    w->volume = atof(node_text_at(n, "volume"));
    w->sample_count = atof(node_text_at(n, "sample_count"));
    wave_put_root_note(w, atoi(node_text_at(n, "root_note")));
    index = atoi(node_text_at(n, "index"));

    s->wave[index] = w;
}

static void parse_wavedata(song_ptr s, node_ptr n)
{
    int index = atoi(node_text_at(n, "index"));
    wave_ptr w = s->wave[index];
    node_ptr n1;
    if (!w) {
	fprintf(stderr, "wave data for nonexistent wave\n");
	return;
    }

    n1 = node_list_at(n, "data");
    if (!n1) {
	fprintf(stderr, "no data field\n");
	return;
    }
    w->data = n1->data;
    w->data_length = n1->len;
    n1->data = NULL;
    n1->len = 0;
}

static void parse_tree(song_ptr s, node_ptr root)
{
    darray_ptr a;
    int i;
    
    a = root->child;
    //create machines
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "machine")) {
	    parse_machine(s, n);
	}
    }
    s->master = song_machine_at(s, node_text_at(root, "master"));
    //create edges
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "connection")) {
	    parse_connection(s, n);
	}
    }
    //create wavetable
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "wave")) {
	    parse_wave(s, n);
	}
    }
    //read wavedata
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "wavedata")) {
	    parse_wavedata(s, n);
	}
    }
}

int song_load(song_ptr s, char *filename)
{
    node_ptr root;
    FILE *fp;

    fp = fopen(filename, "r");
    if (!fp) return 1;

    root = node_new("root");
    tree_read(root, fp);
    fclose(fp);

    song_clear(s);
    song_init(s);
    song_rewind(s);

    parse_tree(s, root);
    tree_free(root);
    return 0;
}
