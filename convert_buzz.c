#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "convert_buzz.h"
#include "song.h"
#include "util.h"
#include "root.h"

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
    char **tdata[track_max];
};
typedef struct buzz_pattern_s *buzz_pattern_ptr;
typedef struct buzz_pattern_s buzz_pattern_t[1];

struct buzz_machine_s {
    char *dll;
    char *id;
    int global_parm_count;
    int track_parm_count;
    int type;
    float x, y;
    machine_ptr blmachine; //bliss counterpart
	//gets assigned during conversion
    int indegree; //needed to compute pattern data size
    int pattern_count;
    int track_count;
    buzz_pattern_ptr pattern;
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

struct buzz_wav_s {
    int index;
    char *filename;
    char *name;
    double volume;
    char flag;
    int level_count;
    buzz_level_ptr level;
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
struct conv_info_s {
    char *dll;
    int global_parm_count;
    int track_parm_count;
    char *bliss_machine;
    void (*convert_pattern)(pattern_ptr p, buzz_machine_ptr bm, buzz_pattern_ptr bp);
};
typedef struct conv_info_s *conv_info_ptr;
typedef struct conv_info_s conv_info_t[1];

static struct conv_info_s cinfo[1024];
static int cinfo_count = 0;

static void add_conv_info(char *dll, int gpc, int tpc, char *m, convertfn f)

{
    conv_info_ptr c = &cinfo[cinfo_count];

    c->dll = strclone(dll);
    c->global_parm_count = gpc;
    c->track_parm_count = tpc;
    c->bliss_machine = m;
    c->convert_pattern = f;

    cinfo_count++;
}

static conv_info_ptr cinfo_at(char *dll)
{
    int i;

    for (i=0; i<cinfo_count; i++) {
	conv_info_ptr c = &cinfo[i];
	if (!strcmp(dll, c->dll)) {
	    return c;
	}
    }
    return NULL;
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
	m->global_parm_count = c->global_parm_count;
	m->track_parm_count = c->track_parm_count;
	return;
    }
    //guess 0
    m->global_parm_count = 0;
    m->track_parm_count = 0;
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
	    printf("UNKNOWN\n");
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
    n = m->global_parm_count;
    if (n) {
	s = (char *) malloc(n);
	fread(s, 1, n, fp);
	free(s);
    }

    //read track count
    n = read_word(fp);

    //read track parameters
    tpsize = m->track_parm_count;
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
    glen = m->global_parm_count;
    tlen = m->track_parm_count;
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

	//TODO: if bit 7 is set...
	//read pos/event tuples
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

	//TODO: if bit 7 is set...

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

static void parse(buzz_song_ptr bs, FILE *fp)
{
    char buf[5];
    int i;
    struct buzz_direntry_s de[buzz_dirmax];
    int de_count;

    buf[4] = 0;
    fread(buf, 1, 4, fp);

    //read marker
    if (strcmp(buf, "Buzz")) {
	fprintf(stderr, "warning: first 4 bytes not 'Buzz'\n");
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
}

static void load(buzz_song_ptr bs, char *filename)
{
    FILE *fp;

    fp = fopen(filename, "rb");
    if (!fp) return;
    parse(bs, fp);
    fclose(fp);
}

static char hexdigit(unsigned char c)
{
    if (c <= 9) {
	return c + '0';
    }
    if (c >= 10 && c <= 15) {
	return c - 10 + 'A';
    }
    return '0';
}

static char *hexshow(unsigned char *s, int n)
{
    char *r = (char *) malloc(n * 2 + 1);
    int i;
    for (i=0; i<n; i++) {
	r[2*i] = hexdigit(s[i] & 0xF);
	r[2*i + 1] = hexdigit(s[i] >> 8);
    }
    r[2*n] = 0;
    return r;
}

static void default_convert(pattern_ptr p,
	buzz_machine_ptr bm, buzz_pattern_ptr bp)
{
    int i, j;
    char *str;
    int glen = bm->global_parm_count;
    int tlen = bm->track_parm_count;

    if (glen) for (j=0; j<bp->rows; j++) {
	str = hexshow(bp->gdata[j], glen);
	pattern_put(p, str, 0, j);
	free(str);
    }

    if (tlen) for (i=0; i<bm->track_count; i++) {
	for (j=0; j<bp->rows; j++) {
	    str = hexshow(bp->tdata[i][j], tlen);
	    pattern_put(p, str, i + 1, j);
	    free(str);
	}
    }
}

void song_convert_buzz(song_ptr s, buzz_song_ptr bs)
{
    int i;

    song_clear(s);
    song_init(s);

    //convert machines
    for (i=0; i<bs->machine_count; i++) {
	buzz_machine_ptr bm = &bs->machine[i];
	machine_ptr m;
	conv_info_ptr ci = cinfo_at(bm->dll);

	if (!ci->bliss_machine) {
	    switch (bm->type) {
		case machine_master:
		    m = song_create_machine(s, "Master", bm->id);
		    break;
		case machine_effect:
		    m = song_create_machine(s, "No Effect", bm->id);
		    break;
		case machine_generator:
		    m = song_create_machine(s, "Alpha Bass 2", bm->id);
		    break;
		default: //bug!
		    m = song_create_machine(s, "No Effect", bm->id);
		    break;
	    }
	} else {
	    m = song_create_machine(s, ci->bliss_machine, bm->id);
	}
	m->x = root_arena_w * (bm->x + 1) * 0.5;
	m->y = root_arena_h * (bm->y + 1) * 0.5;
	bm->blmachine = m;
	if (bm->type == machine_master) {
	    s->master = m;
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
	conv_info_ptr ci = cinfo_at(bm->dll);
	for (j=0; j<bm->pattern_count; j++) {
	    buzz_pattern_ptr bp = &bm->pattern[j];
	    pattern_ptr p = machine_create_pattern(m, bp->id);

	    if (!ci->convert_pattern) {
		default_convert(p, bm, bp);
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
	w->data_length = w->sample_count * 2;
	w->data = bw->level[0].data;
	bw->level[0].data = NULL;
	wave_put_root_note(w, buzz_note_to_note(bw->level[0].root_note));
	song_put_wave(s, w, bw->index);
    }
}

void buzz_machine_clear(buzz_machine_ptr bm)
{
    int i, j;
    free(bm->id);
    free(bm->dll);
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

    //TODO: free wavt stuff
}

int song_import_buzz(song_ptr s, char *filename)
{
    buzz_song_t buzz_song;

    load(buzz_song, filename);
    song_convert_buzz(s, buzz_song);

    buzz_song_clear(buzz_song);
    return 0;
}

static char *to_note(unsigned char c)
{
    static char buf[4];

    static char *scale[12] = {
	"C",
	"C#",
	"D",
	"D#",
	"E",
	"F",
	"F#",
	"G",
	"G#",
	"A",
	"A#",
	"B",
    };

    strcpy(buf, scale[(c % 16) - 1]);
    buf[strlen(buf) + 1] = 0;
    buf[strlen(buf)] = (c / 16) + '0';

    return buf;
}

static char *to_nnote(unsigned char c)
{
    static char buf[5];

    buf[0] = 'n';
    buf[1] = 0;
    strcat(buf, to_note(c));
    return buf;
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

static void cp_Jeskola_Bass_2(pattern_ptr p,
	buzz_machine_ptr bm, buzz_pattern_ptr bp)
{
    int i, j;

    for (i=0; i<bm->track_count; i++) {
	for (j=0; j<bp->rows; j++) {
	    unsigned char c;
	    //col 0 = cutoff
	    c = bp->tdata[i][j][0];
	    convertevent(p, 255, "c", c, 9 * i + 0 + 1, j);
	    //col 1 = resonance
	    c = bp->tdata[i][j][1];
	    convertevent(p, 255, "r", c, 9 * i + 1 + 1, j);
	    //col 5 = note
	    c = bp->tdata[i][j][5];
	    if (c) pattern_put(p, to_nnote(c), 9 * i + 5 + 1, j);
	    //col 6 = volume
	    c = bp->tdata[i][j][6];
	    convertevent(p, 255, "v", c, 9 * i + 7 + 1, j);
	    //col 7 = length
	    c = bp->tdata[i][j][7];
	    convertevent(p, 0, "l", c, 9 * i + 7 + 1, j);
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
	    if (c) pattern_put(p, to_note(c), 5 * i + 0 + 1, j);

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

void convert_buzz_init()
{
    //init conversion tables
    add_conv_info("MASTER", 5, 0, "Master", NULL);
    add_conv_info("Jeskola Bass 2", 0, 9, "Alpha Bass 2", cp_Jeskola_Bass_2);
    add_conv_info("Jeskola EQ-3", 11, 0, NULL, NULL);
    add_conv_info("Jeskola Reverb", 10, 0, NULL, NULL);
    add_conv_info("Jeskola Tracker", 1, 5, "Alpha Tracker", cp_Jeskola_Tracker);
    add_conv_info("Jeskola Delay", 1, 5, NULL, NULL);
    add_conv_info("Jeskola Distortion", 9, 0, NULL, NULL);
}
