#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "song.h"
#include "pattern.h"
#include "mlist.h"
#include "util.h"
#include "base64.h"

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

void song_add_machine(song_ptr s, machine_ptr m)
{
    darray_append(s->machine, m);
    m->song = s;
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

    m = machine_new(mi, id);
    song_add_machine(s, m);
    return m;
}

machine_ptr song_create_machine(song_ptr s, char *gearid, char *id)
{
    machine_ptr m;
    machine_info_ptr mi;

    mi = machine_info_at(gearid);
    if (!mi) return NULL;

    m = machine_new(mi, id);
    song_add_machine(s, m);
    return m;
}

void song_del_machine(song_ptr s, machine_ptr m)
{
    int i, n;
    darray_remove(s->machine, m);
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

void song_init(song_ptr s)
{
    int i;

    darray_init(s->machine);
    darray_init(s->edge);
    for (i=0; i<wave_max; i++) s->wave[i] = NULL;
}

// TODO: move to separate file
// Song I/O functions
// Inefficient, but easy to code

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

static char *argsplit(char *buf)
{
    char *arg;

    arg = strchr(buf, ' ');
    if (arg) {
	*arg = 0;
	arg++;
    }
    return arg;
}

static void node_init(node_ptr n, char *id)
{
    n->leaf_flag = 0;
    n->id = strclone(id);
    darray_init(n->child);
    n->data = NULL;
    n->len = 0;
}

static node_ptr node_new(char *id)
{
    node_ptr n;
    n = (node_ptr) malloc(sizeof(struct node_s));
    node_init(n, id);
    return n;
}

static void node_init_leaf(node_ptr n, char *id, char *text)
{
    n->leaf_flag = 1;
    n->id = strclone(id);
    if (text) n->text = strclone(text);
    n->data = NULL;
    n->len = 0;
}

static node_ptr node_new_leaf(char *id, char *text)
{
    node_ptr n;
    n = (node_ptr) malloc(sizeof(struct node_s));
    node_init_leaf(n, id, text);
    return n;
}

static void node_add(node_ptr n, node_ptr child)
{
    if (n->leaf_flag) {
	//BUG!
    }
    darray_append(n->child, child);
}

static void node_clear(node_ptr n)
{
    if (n->leaf_flag) free(n->text);
    else darray_clear(n->child);
    if (n->data) free(n->data);
    free(n->id);
}

static node_ptr node_at(node_ptr n, char *id)
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

static char *node_leaf_at(node_ptr n, char *id)
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

static void tree_print(node_ptr n)
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

static void tree_free(node_ptr n)
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

void pattern_print(pattern_ptr p, FILE *fp)
{
    cell_ptr c;

    fprintf(fp, "\t\tid %s\n", p->id);
    for (c=p->first->next; c; c=c->next) {
	fprintf(fp, "\t\tcell %d %d %s\n", c->x, c->y, c->text);
    }
}

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
    fprintf(fp, "\tbegin sequence\n");
    track_print(m->track, fp);
    fprintf(fp, "\tend\n");
    for (i=0; i<n; i++) {
	fprintf(fp, "\tbegin pattern\n");
	pattern_ptr p = (pattern_ptr) a->item[i];
	pattern_print(p, fp);
	fprintf(fp, "\tend\n");
    }
    fprintf(fp, "end\n");
}

void song_print(song_ptr s, FILE *fp)
{
    int i, n;
    darray_ptr a;

    fprintf(fp, "bpm %d\n", s->bpm);
    fprintf(fp, "tpb %d\n", s->tpb);
    fprintf(fp, "master %s\n", s->master->id);
    fprintf(fp, "x %d\n", s->x);
    fprintf(fp, "y %d\n", s->y);
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

void skip_whitespace(FILE *fp)
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

static void parse_pattern(pattern_ptr p, node_ptr root)
{
    int i;
    darray_ptr a = root->child;
    char *id = node_leaf_at(root, "id");

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
    char *id = node_leaf_at(root, "id");
    char *plugin = node_leaf_at(root, "plugin");
    machine_ptr m;
    int i;
    darray_ptr a = root->child;

    x = atoi(node_leaf_at(root, "x"));
    y = atoi(node_leaf_at(root, "y"));
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
}

static void parse_connection(song_ptr s, node_ptr n)
{
    char *s1 = node_leaf_at(n, "from");
    char *s2 = node_leaf_at(n, "to");
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

    w->volume = atof(node_leaf_at(n, "volume"));
    w->sample_count = atof(node_leaf_at(n, "sample_count"));
    wave_put_root_note(w, atoi(node_leaf_at(n, "root_note")));
    int index = atoi(node_leaf_at(n, "index"));

    s->wave[index] = w;
}

static void parse_wavedata(song_ptr s, node_ptr n)
{
    int index = atoi(node_leaf_at(n, "index"));
    wave_ptr w = s->wave[index];
    node_ptr n1;
    if (!w) {
	fprintf(stderr, "wave data for nonexistent wave\n");
	return;
    }

    n1 = node_at(n, "data");
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
    int bpm, tpb;
    darray_ptr a;
    int i;
    
    bpm = atoi(node_leaf_at(root, "bpm"));
    tpb = atoi(node_leaf_at(root, "tpb"));
    song_put_bpm_tpb(s, bpm, tpb);
    a = root->child;
    //create machines
    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "machine")) {
	    parse_machine(s, n);
	}
    }
    s->master = song_machine_at(s, node_leaf_at(root, "master"));
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

void song_scan(song_ptr s, FILE *fp)
{
    char buf[1024];
    char *arg;
    node_ptr root;
    node_ptr nodestack[32];
    int sp;

    song_clear(s);
    song_init(s);

    //read into tree
    root = node_new("root");
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
	    node_ptr n = node_new_leaf(buf, arg);
	    node_add(nodestack[sp], n);
	}
    }
    parse_tree(s, root);
    tree_free(root);
}

int song_load(song_ptr s, char *filename)
{
    FILE *fp;

    fp = fopen(filename, "r");
    if (!fp) return 1;
    song_scan(s, fp);
    fclose(fp);
    return 0;
}
