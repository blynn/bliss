#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "convert_buzz.h"
#include "song.h"
#include "util.h"
#include "root.h"
#include "mlist.h"
#include "buzz_machine.h"
#include "parse.h"

enum {
    track_max = 16,
};

struct buzz_event_s {
    int pos;
    int event;
};
typedef struct buzz_event_s *buzz_event_ptr;
typedef struct buzz_event_s buzz_event_t[1];

struct buzz_seq_s {
    int machine;
    int event_count;
    buzz_event_ptr event;
};
typedef struct buzz_seq_s *buzz_seq_ptr;
typedef struct buzz_seq_s buzz_seq_t[1];

struct buzz_pattern_s {
    char *id;
    int rows;
    char **gdata;
    //TODO: buffer overflow
    char **tdata[track_max];
};
typedef struct buzz_pattern_s *buzz_pattern_ptr;
typedef struct buzz_pattern_s buzz_pattern_t[1];

struct conv_info_s;

struct buzz_machine_s {
    char *dll;
    char *id;
    int global_parm_size;
    int track_parm_size;
    int type;
    unsigned char *init_gp; //initial global parameters
    float x, y;
    machine_ptr blmachine; //bliss counterpart
	//gets assigned during conversion
    int indegree; //needed to compute pattern data size
    int pattern_count;
    int track_count;
    buzz_pattern_ptr pattern;
    struct conv_info_s *cinfo;
};

typedef struct buzz_machine_s *buzz_machine_ptr;

struct buzz_edge_s {
    int pan, amp;
    int src, dst;
};

typedef struct buzz_edge_s *buzz_edge_ptr;
typedef struct buzz_edge_s buzz_edge_t[1];

struct buzz_level_s {
    int sample_count;
    int sample_per_sec;
    int loop_begin, loop_end;
    int root_note;
    unsigned char *data;
};
typedef struct buzz_level_s *buzz_level_ptr;
typedef struct buzz_level_s buzz_level_t[1];

struct buzz_point_s {
    int x, y;
    int flag;
};
typedef struct buzz_point_s *buzz_point_ptr;
typedef struct buzz_point_s buzz_point_t[1];

struct buzz_env_s {
    int attack, decay, sustain, release;
    int adsr_sub;
    int adsr_flag;
    int point_count;
    int disabled;
    buzz_point_ptr point;
};
typedef struct buzz_env_s *buzz_env_ptr;
typedef struct buzz_env_s buzz_env_t[1];

struct buzz_wav_s {
    int index;
    char *filename;
    char *name;
    double volume;
    char flag;
    int level_count;
    buzz_level_ptr level;
    int env_count;
    buzz_env_ptr env;
};
typedef struct buzz_wav_s buzz_wav_t[1];
typedef struct buzz_wav_s *buzz_wav_ptr;

struct buzz_song_s {
    int machine_count;
    buzz_machine_ptr machine;
    int edge_count;
    buzz_edge_ptr edge;
    int song_end, loop_begin, loop_end;
    int seq_count;
    buzz_seq_ptr seq;
    int wav_count;
    buzz_wav_ptr wav;
};

typedef struct buzz_song_s *buzz_song_ptr;
typedef struct buzz_song_s buzz_song_t[1];

enum {
    buzz_dirmax = 31,
};

struct buzz_direntry_s {
    char *id;
    int offset;
    int size;
};

typedef struct buzz_direntry_s *buzz_direntry_ptr;

static int buzz_note_to_note(unsigned char c)
{
    return (c % 16) + 12 * (c / 16) - 1;
}

typedef void (*convertfn)(pattern_ptr p, buzz_machine_ptr bm, buzz_pattern_ptr bp);

struct conv_param_s {
    int type;
    int flags;
    int value_min;
    int value_max;
    int value_default;
    int value_no;
    char *id;
    char *desc;
};
typedef struct conv_param_s *conv_param_ptr;
typedef struct conv_param_s conv_param_t[1];

struct conv_info_s {
    char *id;
    int tps, gps;
    int type;
    darray_t gparam;
    darray_t tparam;
    convertfn convert_pattern;
    char *bliss_machine;
};
typedef struct conv_info_s *conv_info_ptr;
typedef struct conv_info_s conv_info_t[1];


darray_t cinfo;

static conv_info_ptr cinfo_at(char *id)
{
    int i;

    for (i=0; i<cinfo->count; i++) {
	conv_info_ptr c = (conv_info_ptr) cinfo->item[i];
	if (!strcmp(id, c->id)) {
	    return c;
	}
    }
    return NULL;
}

static void conv_info_put(char *id, char *bliss_id, convertfn cf)
{
    conv_info_ptr ci;

    ci = cinfo_at(id);
    if (!ci) {
	fprintf(stderr, "No such conv_info in table\n");
	return;
    }
    ci->bliss_machine = bliss_id;
    ci->convert_pattern = cf;
}

static char read_byte(FILE *fp)
{
    return fgetc(fp);
}

static char *read_asciiz(FILE *fp)
{
    char c;
    char *s;
    int count = 0, max = 8;

    s = malloc(max);
    for (;;) {
	s[count] = c = read_byte(fp);
	if (!c) break;
	count++;
	if (count == max) {
	    max *= 2;
	    s = (char *) realloc(s, max);
	}
    }
    return s;
}

static int read_bytes(FILE *fp, int n)
{
    int i;
    unsigned char *buf, *s;
    buf = (unsigned char *) alloca(n);
    fread(buf, 1, n, fp);

    i = 0;

    for (s = &buf[n - 1]; s >= buf; s--) {
	i <<= 8;
	i |= *s;
    }
    return i;
}

static int read_word(FILE *fp)
{
    int i;
    unsigned char buf[2];
    fread(buf, 1, 2, fp);

    i = *buf;
    i += buf[1] << 8;
    return i;
}

static int read_dword(FILE *fp)
{
    int i;
    unsigned char buf[4], *s;
    fread(buf, 1, 4, fp);

    s = buf;
    i = *s;
    s++;
    i |= *s << 8;
    s++;
    i |= *s << 16;
    s++;
    i |= *s << 24;
    return i;
}

static float read_float(FILE *fp)
{
    int n = read_dword(fp);
    return *((float *) (&n));
}

static void put_parm_count(buzz_machine_ptr m, char *dll)
{
    conv_info_ptr c = cinfo_at(dll);

    if (c) {
	m->cinfo = c;
	m->global_parm_size = c->gps;
	m->track_parm_size = c->tps;
	return;
    } else {
	fprintf(stderr, "unhandled machine: %s\n", dll);
	exit(1);
    }
}

static void parse_machine(buzz_song_ptr bs, buzz_machine_ptr m, FILE *fp)
{
    int i, n;
    int k;
    int tpsize;
    char *s;
    char c;

    //read name
    s = read_asciiz(fp);
    m->id = strclone(s);
    free(s);

    //read machine type
    c = read_byte(fp);
    switch(c) {
	case 0:
	    m->type = machine_master;
	    break;
	case 1:
	    m->type = machine_generator;
	    break;
	case 2:
	    m->type = machine_effect;
	    break;
	default:
	    fprintf(stderr, "%x: unknown machine type!\n", (int) ftell(fp) - 1);
	    exit(1);
	    break;
    }
    if (c) {
	//read DLL if it's a gen or eff
	s = read_asciiz(fp);
	m->dll = strclone(s);
	free(s);
    } else {
	//type = master means no DLL
	//in this case set it to magic string
	m->dll = strclone("MASTER");
    }

    //look up parameter counts in conversion table
    put_parm_count(m, m->dll);

    //read coords
    m->x = read_float(fp);
    m->y = read_float(fp);

    //read machine-specific data
    n = read_dword(fp);
    s = (char *) malloc(n);
    fread(s, 1, n, fp);
    free(s);

    //read attributes
    n = read_word(fp);
    for (i=0; i<n; i++) {
	s = read_asciiz(fp);
	free(s);
	k = read_dword(fp);
    }

    //read global parameters
    n = m->global_parm_size;
    if (n) {
	s = (char *) malloc(n);
	fread(s, 1, n, fp);
	m->init_gp = s;
    } else m->init_gp = NULL;

    //read track count
    n = read_word(fp);

    //read track parameters
    tpsize = m->track_parm_size;
    if (tpsize) {
	for (i=0; i<n; i++) {
	    s = (char *) malloc(tpsize);
	    fread(s, 1, tpsize, fp);
	    free(s);
	}
    }

    //init indegree
    m->indegree = 0;
}

static void parse_machine_section(buzz_song_ptr bs, FILE *fp)
{
    int i;
    bs->machine_count = read_word(fp);
    bs->machine = (buzz_machine_ptr)
	malloc(bs->machine_count * sizeof(struct buzz_machine_s));
    for (i=0; i<bs->machine_count; i++) {
	parse_machine(bs, &bs->machine[i], fp);
    }
}

static void parse_pattern_data(buzz_pattern_ptr p, buzz_machine_ptr m, FILE *fp)
{
    int i, j;
    int glen, tlen;
    char *buf;

    char *str;
    p->id = strclone(str = read_asciiz(fp));
    free(str);
    p->rows = read_word(fp);

    p->gdata = (char **) malloc(p->rows * sizeof(char *));
    for (i=0; i<m->track_count; i++) {
	p->tdata[i] = (char **) malloc(p->rows * sizeof(char *));
    }
    glen = m->global_parm_size;
    tlen = m->track_parm_size;
    buf = (char *) alloca(glen + tlen);

    for (i=0; i<m->indegree; i++) {
	//in?! no idea
	read_word(fp);
	for (j=0; j<p->rows; j++) {
	    //amp
	    read_word(fp);
	    //pan
	    read_word(fp);
	}
    }

    if (glen) for (j=0; j<p->rows; j++) {
	p->gdata[j] = (char *) malloc(glen);
	fread(p->gdata[j], 1, glen, fp);
    }

    if (tlen) for (i=0; i<m->track_count; i++) {
	for (j=0; j<p->rows; j++) {
	    p->tdata[i][j] = (char *) malloc(tlen);
	    fread(p->tdata[i][j], 1, tlen, fp);
	}
    }
}

static void parse_pattern_section(buzz_song_ptr bs, FILE *fp)
{
    int i, j;
    for (i=0; i<bs->machine_count; i++) {
	buzz_machine_ptr m = &bs->machine[i];
	m->pattern_count = read_word(fp);
	m->pattern = (buzz_pattern_ptr) malloc(m->pattern_count * sizeof(buzz_pattern_t));
	m->track_count = read_word(fp);
	for (j=0; j<m->pattern_count; j++) {
	    buzz_pattern_ptr p = &m->pattern[j];
	    parse_pattern_data(p, m, fp);
	}
    }
}

static void parse_sequence_section(buzz_song_ptr bs, FILE *fp)
{
    int i;
    bs->song_end = read_dword(fp);
    bs->loop_begin = read_dword(fp);
    bs->loop_end = read_dword(fp);
    bs->seq_count = read_word(fp);

    bs->seq = (buzz_seq_ptr) malloc(bs->seq_count * sizeof(buzz_seq_t));

    for (i=0; i<bs->seq_count; i++) {
	int bpe, bppos;
	int j;
	buzz_seq_ptr s = &bs->seq[i];
	s->machine = read_word(fp);
	s->event_count = read_dword(fp);
	s->event = (buzz_event_ptr) malloc(s->event_count * sizeof(buzz_event_t));
	//read bytes per pos
	bppos = read_byte(fp);

	//read bytes per event
	bpe = read_byte(fp);

	//read pos/event tuples
	//TODO: handle mute, break
	for (j=0; j<s->event_count; j++) {
	    s->event[j].pos = read_bytes(fp, bppos);
	    s->event[j].event = read_bytes(fp, bpe);
	}
    }
}

static void parse_connection_section(buzz_song_ptr bs, FILE *fp)
{
    int i;
    bs->edge_count = read_word(fp);
    bs->edge = (buzz_edge_ptr) malloc(bs->edge_count * sizeof(buzz_edge_t));
    for (i=0; i<bs->edge_count; i++) {
	buzz_edge_ptr e = &bs->edge[i];
	e->src = read_word(fp);
	e->dst = read_word(fp);
	e->amp = read_word(fp);
	e->pan = read_word(fp);
	bs->machine[e->dst].indegree++;
    }
}

static void parse_env(buzz_env_ptr e, FILE *fp)
{
    int i;

    e->attack = read_word(fp);
    e->decay = read_word(fp);
    e->sustain = read_word(fp);
    e->release = read_word(fp);
    e->adsr_sub = read_byte(fp);
    e->adsr_flag = read_byte(fp);
    e->point_count = read_word(fp);
    if (e->point_count & 0x8000) {
	e->point_count &= 0x7FFF;
	e->disabled = 1;
    } else {
	e->disabled = 0;
    }
    if (e->point_count) {
	e->point = (buzz_point_ptr) malloc(sizeof(buzz_point_t) * e->point_count);
	for (i=0; i<e->point_count; i++) {
	    e->point[i].x = read_word(fp);
	    e->point[i].y = read_word(fp);
	    e->point[i].flag = read_byte(fp);
	}
    } else {
	e->point = NULL;
    }
}

static void parse_wavt_section(buzz_song_ptr bs, FILE *fp)
{
    int i, j;
    bs->wav_count = read_word(fp);
    bs->wav = (buzz_wav_ptr) malloc(bs->wav_count * sizeof(buzz_wav_t));
    for (i=0; i<bs->wav_count; i++) {
	buzz_wav_ptr w = &bs->wav[i];
	char *s;
	//read index, filename, name
	//Buzz numbers waves from 1, so add 1
	w->index = read_word(fp) + 1;

	s = read_asciiz(fp);
	w->filename = strclone(s);
	free(s);

	s = read_asciiz(fp);
	w->name = strclone(s);
	free(s);

	//read volume, flags
	w->volume = read_float(fp);
	w->flag = read_byte(fp);

	if (w->flag & 0x80) {
	    w->env_count = read_word(fp);
	    w->env = (buzz_env_ptr) malloc(w->env_count * sizeof(buzz_env_t));
	    for (j=0; j<w->env_count; j++) {
		parse_env(&w->env[j], fp);
	    }
	} else {
	    w->env = NULL;
	    w->env_count = 0;
	}

	//read levels
	w->level_count = read_byte(fp);
	w->level = (buzz_level_ptr) malloc(w->level_count * sizeof(buzz_level_t));
	for (j=0; j<w->level_count; j++) {
	    buzz_level_ptr l = &w->level[j];
	    l->sample_count = read_dword(fp);
	    l->loop_begin = read_dword(fp);
	    l->loop_end = read_dword(fp);
	    l->sample_per_sec = read_dword(fp);
	    l->root_note = read_byte(fp);
	    l->data = NULL;
	}
    }
}

static void parse_wave_section(buzz_song_ptr bs, FILE *fp)
{
    int i, j;
    int n = read_word(fp);
    for (i=0; i<n; i++) {
	//Buzz numbers waves from 1, so add 1
	int index = read_word(fp) + 1;
	buzz_wav_ptr w = NULL;
	//find wav with matching index
	for (j=0; j<bs->wav_count; j++) {
	    w = &bs->wav[j];
	    if (w->index == index) break;
	}
	if (w->index != index) {
	    fprintf(stderr, "no such wave!\n");
	    exit(1);
	}

	if (read_byte(fp)) {
	    fprintf(stderr, "unknown wave format!\n");
	    exit(1);
	}
	//read #bytes in all levels
	read_dword(fp);
	for (j=0; j<w->level_count; j++) {
	    buzz_level_ptr l = &w->level[j];
	    //16-bit wav, hence the 2
	    l->data = (unsigned char *) malloc(2 * l->sample_count);
	    fread(l->data, 2, l->sample_count, fp);
	}
    }
}

static int parse(buzz_song_ptr bs, FILE *fp)
{
    char buf[5];
    int i;
    struct buzz_direntry_s de[buzz_dirmax];
    int de_count;

    buf[4] = 0;
    fread(buf, 1, 4, fp);

    //read marker
    if (strcmp(buf, "Buzz")) {
	fprintf(stderr, "error: first 4 bytes not 'Buzz'\n");
	return 1;
    }

    //read direntries (up to 31 of these)
    de_count = read_dword(fp);
    for (i=0; i<de_count; i++) {
	fread(buf, 1, 4, fp);
	de[i].id = strclone(buf);
	de[i].offset = read_dword(fp);
	de[i].size = read_dword(fp);
    }

    for (i=0; i<de_count; i++) {
	fseek(fp, de[i].offset, SEEK_SET);
	if (!strcmp(de[i].id, "MACH")) {
	    parse_machine_section(bs, fp);
	} else if (!strcmp(de[i].id, "CONN")) {
	    parse_connection_section(bs, fp);
	} else if (!strcmp(de[i].id, "PATT")) {
	    parse_pattern_section(bs, fp);
	} else if (!strcmp(de[i].id, "SEQU")) {
	    parse_sequence_section(bs, fp);
	} else if (!strcmp(de[i].id, "WAVT")) {
	    parse_wavt_section(bs, fp);
	} else if (!strcmp(de[i].id, "WAVE")) {
	    parse_wave_section(bs, fp);
	}
    }
    return 0;
}

static int load(buzz_song_ptr bs, char *filename)
{
    FILE *fp;
    int status;

    fp = fopen(filename, "rb");
    if (!fp) return 1;
    status = parse(bs, fp);
    fclose(fp);
    return status;
}

void buzz_machine_clear(buzz_machine_ptr bm)
{
    int i, j;
    free(bm->id);
    free(bm->dll);
    free(bm->init_gp);
    for (i=0; i<bm->pattern_count; i++) {
	buzz_pattern_ptr p = &bm->pattern[i];
	for (j=0; j<bm->track_count; j++) {
	    free(p->tdata[j]);
	}
	free(p->gdata);
	free(p->id);
    }
}

void buzz_seq_clear(buzz_seq_ptr s)
{
    free(s->event);
}

void buzz_env_free(buzz_env_ptr be)
{
    if (be->point_count) free(be->point);
}

void buzz_wav_free(buzz_wav_ptr bw)
{
    int i;
    free(bw->filename);
    free(bw->name);
    for (i=0; i<bw->env_count; i++) {
	buzz_env_free(&bw->env[i]);
    }
    free(bw->env);

    for (i=0; i<bw->level_count; i++) {
	//don't free level data
	//the bliss song points to it
	//rather than making another copy
    }
    free(bw->level);
}

void buzz_song_clear(buzz_song_ptr bs)
{
    int i;

    //free machines
    for (i=0; i<bs->machine_count; i++) {
	buzz_machine_clear(&bs->machine[i]);
    }
    free(bs->machine);
 
    //free edges
    free(bs->edge);

    //free sequences
    for (i=0; i<bs->seq_count; i++) {
	buzz_seq_clear(&bs->seq[i]);
    }
    free(bs->seq);

    //free wavt stuff
    for (i=0; i<bs->wav_count; i++) {
	buzz_wav_free(&bs->wav[i]);
    }
    free(bs->wav);
}

static void convertevent(pattern_ptr p, int zerovalue,
	char *cmd, unsigned char c, int x, int y)
{
    char buf[16];
    int i = (int) c;
    if (i != zerovalue) {
	sprintf(buf, "%s%02X", cmd, i);
	pattern_put(p, buf, x, y);
    }
}

static int buzz_convert_raw_param_data(char *buf, conv_param_ptr par,
	unsigned char *in)
{
    int i;
    *buf = 0;
    if (par->type == pt_word) {
	i = *in | in[1] << 8;
	if (par->value_no == i) return 2;
	sprintf(buf, "%X", i);
	return 2;
    }

    i = *in;
    if (par->value_no == i) return 1;
    switch(par->type) {
	case pt_switch:
	case pt_byte:
	    sprintf(buf, "%X", i);
	    break;
	case pt_note:
	    if (!i) return 1;
	    i = (i % 16) + 12 * (i / 16) - 1;
	    strcpy(buf, note_to_text(i));
	    break;
    }
    return 1;
}

static void buzz_convert_pattern(pattern_ptr p,
	buzz_machine_ptr bm, buzz_pattern_ptr bp)
{
    int i, j, k;
    int col;

    conv_info_ptr ci = bm->cinfo;

    for (j=0; j<bp->rows; j++) {
	char *c = bp->gdata[j];
	col = 0;
	for (k=0; k<ci->gparam->count; k++) {
	    char text[8];
	    conv_param_ptr par;

	    par = ci->gparam->item[k];
	    c += buzz_convert_raw_param_data(text, par, c);
	    if (*text) {
		pattern_put(p, text, col, j);
	    }
	    col++;
	}
    }
    for (i=0; i<bm->track_count; i++) {
	for (j=0; j<bp->rows; j++) {
	    char *c = bp->tdata[i][j];
	    col = bm->global_parm_size;
	    for (k=0; k<ci->tparam->count; k++) {
		char text[8];
		conv_param_ptr par;

		par = ci->tparam->item[k];
		c += buzz_convert_raw_param_data(text, par, c);
		if (*text) {
		    pattern_put(p, text, col, j);
		}
		col++;
	    }
	}
    }
}

static void cp_Jeskola_Tracker(pattern_ptr p,
	buzz_machine_ptr bm, buzz_pattern_ptr bp)
{
    int i, j;

    for (i=0; i<bm->track_count; i++) {
	for (j=0; j<bp->rows; j++) {
	    unsigned char c;
	    //col 0 = note
	    c = bp->tdata[i][j][0];
	    if (c) pattern_put(p, note_to_text(buzz_note_to_note(c)), 5 * i + 0 + 1, j);

	    //col 1 = sample
	    c = bp->tdata[i][j][1];
	    convertevent(p, 0, "", c, 5 * i + 1 + 1, j);

	    //col 2 = volume
	    c = bp->tdata[i][j][2];
	    convertevent(p, 255, "", c, 5 * i + 2 + 1, j);

	    //col 3 = effect
	    c = bp->tdata[i][j][3];
	    if (c) {
		convertevent(p, 0, "", c, 5 * i + 3 + 1, j);

		//col 4 = effect_data
		c = bp->tdata[i][j][4];
		convertevent(p, -1, "", c, 5 * i + 4 + 1, j);
	    }
	}
    }
}

static void master_init_state(machine_ptr m, buzz_machine_ptr bm)
{
    int vol;
    int bpm;
    int tpb;
    char buf[8];

    vol = bm->init_gp[0] + 256 * bm->init_gp[1];
    bpm = bm->init_gp[2] + 256 * bm->init_gp[3];
    tpb = bm->init_gp[4];
    if (bpm != 0xFFFF) {
	cell_t c;
	sprintf(buf, "b%x", bpm);
	machine_cell_init(c, m, buf, -1);
	machine_parse(m, c, -1);
	cell_clear(c);
    }
    if (tpb != 0xFF) {
	cell_t c;
	sprintf(buf, "t%x", tpb);
	machine_cell_init(c, m, buf, -1);
	machine_parse(m, c, -1);
	cell_clear(c);
    }
}

void song_convert_buzz(song_ptr s, buzz_song_ptr bs)
{
    int i;

    song_clear(s);
    song_init(s);

    //convert some global data
    s->song_end = bs->song_end;
    s->loop_end = bs->loop_end;
    s->loop_begin = bs->loop_begin;

    //convert machines
    for (i=0; i<bs->machine_count; i++) {
	buzz_machine_ptr bm = &bs->machine[i];
	conv_info_ptr ci = bm->cinfo;
	machine_ptr m;

	if (!ci->bliss_machine) {
	    switch (bm->type) {
		case machine_master:
		    m = song_create_machine(s, "Master", bm->id);
		    break;
		case machine_effect:
		    m = song_create_machine(s, "No Effect", bm->id);
		    break;
		case machine_generator:
		    m = song_create_machine(s, "No Effect", bm->id);
		    break;
		default: //bug!
		    m = song_create_machine(s, "No Effect", bm->id);
		    break;
	    }
	} else {
	    m = song_create_machine(s, ci->bliss_machine, bm->id);
	}
	root_put_buzz_coord(m, bm->x, bm->y);
	bm->blmachine = m;
	if (bm->type == machine_master) {
	    s->master = m;
	}
	//convert and apply init state
	//TODO: replace this kludge with real code
	if (bm->type == machine_master) {
	    master_init_state(m, bm);
	}
    }

    //convert edges
    for (i=0; i<bs->edge_count; i++) {
	buzz_edge_ptr be = &bs->edge[i];
	machine_ptr src, dst;

	src = bs->machine[be->src].blmachine;
	dst = bs->machine[be->dst].blmachine;
	song_create_edge(s, src, dst);
    }

    //convert patterns
    for (i=0; i<bs->machine_count; i++) {
	int j;
	buzz_machine_ptr bm = &bs->machine[i];
	machine_ptr m = bm->blmachine;
	conv_info_ptr ci = bm->cinfo;
	for (j=0; j<bm->pattern_count; j++) {
	    buzz_pattern_ptr bp = &bm->pattern[j];
	    pattern_ptr p = machine_create_pattern(m, bp->id);

	    if (!ci->convert_pattern) {
		buzz_convert_pattern(p, bm, bp);
	    } else {
		ci->convert_pattern(p, bm, bp);
	    }
	}
    }

    //convert sequences
    for (i=0; i<bs->seq_count; i++) {
	buzz_seq_ptr s = &bs->seq[i];
	buzz_machine_ptr bm = &bs->machine[s->machine];
	machine_ptr m = bm->blmachine;
	int j;

	for (j=0; j<s->event_count; j++) {
	    buzz_event_ptr be = &s->event[j];

	    //0x00 = mute, 0x01 = break, 0x02 = thru
	    //0x10 and above are the patterns
	    if (be->event >= 0x10) {
		track_put(m->track,
		bm->pattern[be->event - 0x10].id,
			be->pos);
	    }
	}
    }

    //convert waves
    for (i=0; i<bs->wav_count; i++) {
	buzz_wav_ptr bw = &bs->wav[i];
	wave_ptr w = wave_new();
	if (bw->level_count > 1) {
	    fprintf(stderr, "ignoring extra levels\n");
	}
	w->id = strclone(bw->name);
	w->volume = bw->volume;
	w->sample_count = bw->level[0].sample_count;
	w->sample_rate = bw->level[0].sample_per_sec;
	w->data_length = w->sample_count * 2;
	w->data = bw->level[0].data;
	bw->level[0].data = NULL;
	wave_put_root_note(w, buzz_note_to_note(bw->level[0].root_note));
	song_put_wave(s, w, bw->index);
    }
}

int song_import_buzz(song_ptr s, char *filename)
{
    buzz_song_t buzz_song;

    if (load(buzz_song, filename)) return 1;

    song_convert_buzz(s, buzz_song);

    buzz_song_clear(buzz_song);

    song_rewind(s);
    return 0;
}

int parse_conv_param(conv_param_ptr p, node_ptr root)
{
    p->type = atoi(node_text_at(root, "type"));
    p->value_no = strtol(node_text_at(root, "no"), NULL, 0);
    if (p->type == pt_word) return 2;
    return 1;
}

void parse_conv_machine(node_ptr root)
{
    int i, n;
    darray_ptr a;
    conv_info_ptr c;
    static int conv_type[] = {2, 1, 3};

    c = (conv_info_ptr) malloc(sizeof(conv_info_t));
    darray_append(cinfo, c);
    c->id = node_text_at(root, "dll");
    *strchr(c->id, '.') = 0;
    c->type = conv_type[atoi(node_text_at(root, "type"))];
    c->gps = 0;
    c->tps = 0;
    c->convert_pattern = NULL;
    darray_init(c->gparam);
    darray_init(c->tparam);

    a = root->child;
    n = a->count;
    for (i=0; i<n; i++) {
	node_ptr n1 = a->item[i];
	conv_param_ptr p;
	if (!n1->leaf_flag && !strcmp(n1->id, "gparam")) {
	    p = (conv_param_ptr) malloc(sizeof(conv_param_t));
	    darray_append(c->gparam, p);
	    c->gps += parse_conv_param(p, n1);
	}
	if (!n1->leaf_flag && !strcmp(n1->id, "tparam")) {
	    p = (conv_param_ptr) malloc(sizeof(conv_param_t));
	    darray_append(c->tparam, p);
	    c->tps += parse_conv_param(p, n1);
	}
    }

    //if there's a plugin with same machine id, note this
    if (machine_info_at(c->id)) {
	c->bliss_machine = c->id;
    } else {
	c->bliss_machine = NULL;
    }
}

void parse_conv_tree(node_ptr root)
{
    int i;
    darray_ptr a = root->child;

    for (i=0; i<a->count; i++) {
	node_ptr n = a->item[i];
	if (!n->leaf_flag && !strcmp(n->id, "machine")) {
	    parse_conv_machine(n);
	}
    }
}

void convert_buzz_init()
{
    FILE *fp;
    node_ptr root;

    darray_init(cinfo);

    fp = fopen("bminfo.txt", "r");
    if (!fp) {
	fprintf(stderr, "Can't load Buzz conversion table!\n");
	return;
    }
    root = node_new("root");
    tree_read(root, fp);
    fclose(fp);

    parse_conv_tree(root);
    tree_free(root);

    //some manual conversions
    //conv_info_put("MASTER", "Master", NULL);
    conv_info_put("Jeskola Tracker", "Alpha Tracker", cp_Jeskola_Tracker);
}
