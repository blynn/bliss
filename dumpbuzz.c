//Use with something like:
//for f in *.dll; do dumpbuzz "$f"; done
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

enum {
    pt_note = 0,
    pt_switch = 1,
    pt_byte = 2,
    pt_word = 3,
};

struct param_s {
    int type;
    int flags;
    int value_min;
    int value_max;
    int value_default;
    int value_no;
    char *id;
    char *desc;
};

struct macinfo_s {
    int type;
    int version;
    int flags;
    int track_min;
    int track_max;
    int gpc, tpc;
    char *dll;
    char *name, *id, *author;
    struct param_s *gparam;
    struct param_s *tparam;
} macinfo;

struct sec_s {
    char name[16];
    int vsize;
    int rva;
    int rawsize;
    int rawoffset;
};

struct sec_s *sec;
int section_count;

char* read_asciiz(FILE *fp)
{
    char *buf;
    int i;
    buf = (char *) malloc(100);

    //TODO: fix buffer overflow:
    for (i=0; (buf[i] = fgetc(fp)); i++);
    buf = (char *) realloc(buf, strlen(buf) + 1);
    return buf;
}

int read_word(FILE *fp)
{
    unsigned char buf[2];
    int i;
    int r = 0;
    fread(buf, 1, 2, fp);
    for (i=0; i<2; i++) {
	r |= (buf[i] << 8 * i);
    }
    return r;
}

long read_long(FILE *fp)
{
    unsigned char buf[4];
    int i;
    long l = 0;
    fread(buf, 1, 4, fp);
    for (i=0; i<4; i++) {
	l |= (buf[i] << 8 * i);
    }
    return l;
}

int read_dword(FILE *fp)
{
    return read_long(fp);
}

int read_ptr(FILE *fp)
{
    return read_dword(fp) & 0x0FFFFFFF;
}

void rva_seek(FILE *fp, int rva)
{
    int i;
    for (i=0; i<section_count; i++) {
	if (rva < sec[i].rva) break;
    }
    i--;
    if (i < 0) {
	fprintf(stderr, "invalid rva!\n");
	exit(1);
    }
    fseek(fp, sec[i].rawoffset, SEEK_SET);
    fseek(fp, rva - sec[i].rva, SEEK_CUR);
}

int export_base;
int export_count;
int export_name_count;
int export_rva;
int export_size;

struct export_func_s {
    int ordinal;
    int rva;
};

struct export_func_s *export_address_func;
int *export_address_name;
int *export_address_ordinal;

int export_af_rva;
int export_an_rva;
int export_ao_rva;

int lookup_export_by_ordinal(FILE *fp, int ord)
{
    int i;
    for (i=0; i<export_count; i++) {
	if (export_address_func[i].ordinal == ord) {
	    return export_address_func[i].rva;
	}
    }
    return 0;
}

int lookup_export_by_name(FILE *fp, char *s)
{
    int len = strlen(s);
    char *buf = (char *) alloca(len + 2);
    int i;
    buf[len + 1] = 0;
    for (i=0; i<export_name_count; i++) {
	rva_seek(fp, export_address_name[i]);
	fread(buf, 1, len + 1, fp);
	if (!strcmp(buf, s)) {
	    //PE specs say you should do this: but they appear to be wrong!
	    //int ord = export_address_ordinal[i];
	    //return lookup_export_by_ordinal(fp, ord);
	    return export_address_func[i].rva;
	}
    }
    return 0;
}

void readparam(struct param_s *p, FILE *fp)
{
    int id_rva, desc_rva;
    int jumprva = read_ptr(fp);
    int saverva = ftell(fp);
    rva_seek(fp, jumprva);
    p->type = read_dword(fp);
    id_rva = read_ptr(fp);
    desc_rva = read_ptr(fp);
    p->value_min = read_dword(fp);
    p->value_max = read_dword(fp);
    p->value_no = read_dword(fp);
    p->flags = read_dword(fp);
    p->value_default = read_dword(fp);
    rva_seek(fp, desc_rva);
    p->desc = read_asciiz(fp);
    rva_seek(fp, id_rva);
    p->id = read_asciiz(fp);
    fseek(fp, saverva, SEEK_SET);
}

void readmacinfo(struct macinfo_s *m, FILE *fp)
{
    int param_rva, attr_rva;
    int id_rva, name_rva, author_rva;
    int f_rva = lookup_export_by_name(fp, "GetInfo");
    int i;
    if (!f_rva) {
	fprintf(stderr, "GetInfo not found!\n");
	exit(1);
    }

    rva_seek(fp, f_rva);

    if (fgetc(fp) != 0xB8) {
	fprintf(stderr, "mov eax instruction expected\n");
	exit(1);
    }

    rva_seek(fp, read_ptr(fp));

    //should be at MacInfo struct now
    m->type = read_dword(fp);
    m->version = read_dword(fp);
    m->flags = read_dword(fp);
    m->track_min = read_dword(fp);
    m->track_max = read_dword(fp);
    m->gpc = read_dword(fp);
    m->tpc = read_dword(fp);

    param_rva = read_ptr(fp);

    read_dword(fp); //not sure what this field is
    attr_rva = read_ptr(fp);
    id_rva = read_ptr(fp);
    name_rva = read_ptr(fp);
    author_rva = read_ptr(fp);

    rva_seek(fp, id_rva);
    m->id = read_asciiz(fp);
    rva_seek(fp, name_rva);
    m->name = read_asciiz(fp);
    rva_seek(fp, author_rva);
    m->author = read_asciiz(fp);

    if (m->gpc || m->tpc) {
	m->gparam = (struct param_s *) malloc(sizeof(struct param_s) * m->gpc);
	m->tparam = (struct param_s *) malloc(sizeof(struct param_s) * m->tpc);

	rva_seek(fp, param_rva);
	for (i=0; i<m->gpc; i++) {
	    readparam(&m->gparam[i], fp);
	}

	for (i=0; i<m->tpc; i++) {
	    readparam(&m->tparam[i], fp);
	}
    }
}

static int tablevel = 0;

static void tabinc()
{
    tablevel++;
}

static void tabdec()
{
    tablevel--;
}

static int tabprintf(char *s, ...)
{
    va_list ap;
    int i;
    int status;
    va_start(ap, s);
    for (i=0; i<tablevel; i++) {
	putchar(' ');
	putchar(' ');
    }
    status = tablevel * 2 + vprintf(s, ap);
    va_end(ap);
    return status;
}

static void print_param(struct param_s *p)
{
    tabprintf("type %d\n", p->type);
    tabprintf("id %s\n", p->id);
    tabprintf("desc %s\n", p->desc);
    tabprintf("min %d\n", p->value_min);
    tabprintf("max %d\n", p->value_max);
    tabprintf("no %d\n", p->value_no);
    tabprintf("flags %d\n", p->flags);
    tabprintf("default %d\n", p->value_default);
}

static void printconvertinfo(struct macinfo_s *m)
{
    int i;

    tabprintf("begin machine\n");
    tabinc();
    tabprintf("dll %s\n", m->dll);
    tabprintf("type %d\n", m->type);
    for (i=0; i<m->gpc; i++) {
	struct param_s *p = &m->gparam[i];
	tabprintf("begin gparam\n");
	tabinc();
	print_param(p);
	tabdec();
	tabprintf("end gparam\n");
    }
    for (i=0; i<m->tpc; i++) {
	struct param_s *p = &m->tparam[i];
	tabprintf("begin tparam\n");
	tabinc();
	print_param(p);
	tabdec();
	tabprintf("end tparam\n");
    }
    tabdec();
    tabprintf("end machine\n");
}

int main(int argc, char **argv)
{
    FILE *fp;
    unsigned char buf[1024];
    int pe_offset;
    int opsize;
    int i;
    int dll_rva;

    if (argc < 2) {
	printf("No Buzz dll supplied\n");
	exit(0);
    }
    fp = fopen(argv[1], "r");

    if (!fp) {
	fprintf(stderr, "error opening file\n");
	exit(1);
    }

    fread(buf, 1, 2, fp);
    if (strncmp(buf, "MZ", 2)) {
	fprintf(stderr, "magic missing\n");
	exit(1);
    }
    //look for PE header offset
    fseek(fp, 60, SEEK_SET);
    pe_offset = read_long(fp);

    //seek to PE header
    fseek(fp, pe_offset, SEEK_SET);
    fread(buf, 1, 4, fp);
    if (strncmp(buf, "PE\0\0", 4)) {
	fprintf(stderr, "signature missing\n");
	exit(1);
    }

    //get section count
    fseek(fp, 2, SEEK_CUR);
    section_count = read_word(fp);

    //get optional header size
    fseek(fp, 12, SEEK_CUR);
    opsize = read_word(fp);

    //seek past last 2 bytes of IMAGE_FILE_HEADER
    fseek(fp, 2, SEEK_CUR);

    //seek past most of "optional" header
    fseek(fp, opsize - 16 * 8, SEEK_CUR);

    //should be at export dir entry
    export_rva = read_dword(fp);
    export_size = read_dword(fp);

    //don't care about other dir entries
    fseek(fp, 15 * 8, SEEK_CUR);

    //should be at section table
    sec = (struct sec_s *) malloc(sizeof(struct sec_s) * section_count);

    for (i=0; i<section_count; i++) {
	struct sec_s *p = &sec[i];
	fread(p->name, 1, 8, fp);
	p->name[8] = 0; //not guaranteed to be NULL terminated
	p->vsize = read_dword(fp);
	p->rva = read_dword(fp);
	p->rawsize = read_dword(fp);
	p->rawoffset = read_dword(fp);
	//ignore other fields
	fseek(fp, 16, SEEK_CUR);
    }

    //now locate exports
    rva_seek(fp, export_rva);

    //seek over characteristics, stamp, version
    fseek(fp, 12, SEEK_CUR);

    dll_rva = read_dword(fp);

    export_base = read_dword(fp);

    export_count = read_dword(fp);

    export_name_count = read_dword(fp);

    export_af_rva = read_dword(fp);
    export_an_rva = read_dword(fp);
    export_ao_rva = read_dword(fp);

    export_address_func = (struct export_func_s *) malloc(sizeof(struct export_func_s) * export_count);
    export_address_name = (int *) malloc(sizeof(int) * export_name_count);
    export_address_ordinal = (int *) malloc(sizeof(int) * export_name_count);

    //read AddressOfFunctions
    rva_seek(fp, export_af_rva);
    for (i=0; i<export_count; i++) {
	export_address_func[i].rva = read_dword(fp);
	export_address_func[i].ordinal = export_base + i;
    }

    //read AddressOfNames
    rva_seek(fp, export_an_rva);
    for (i=0; i<export_name_count; i++) {
	export_address_name[i] = read_dword(fp);
    }

    //read AddressOfOrdinals
    rva_seek(fp, export_ao_rva);
    for (i=0; i<export_name_count; i++) {
	export_address_ordinal[i] = read_word(fp);
    }

    rva_seek(fp, dll_rva);
    macinfo.dll = read_asciiz(fp);

    readmacinfo(&macinfo, fp);

    printconvertinfo(&macinfo);

    fclose(fp);
    return 0;
}
