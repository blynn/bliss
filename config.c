#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "darray.h"
#include "util.h"

#define CONFIGFILE "config.txt"

struct parm_s {
    char *key, *value;
};

typedef struct parm_s parm_t[1];
typedef struct parm_s *parm_ptr;

static darray_t parm_table;
void config_init()
{
    FILE *fp;
    char s[80];
    parm_ptr p;

    void read_word()
    {
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

    darray_init(parm_table);

    fp = fopen(CONFIGFILE, "rb");
    if (!fp) return;
    read_word();
    for (;;) {
	if (!*s) break;
	p = malloc(sizeof(parm_t));
	p->key = strclone(s);
	read_word();
	p->value = strclone(s);
	darray_append(parm_table, p);
	read_word();
    }

    fclose(fp);
}

char *config_at(char *key)
{
    int i;
    for (i=0; i<parm_table->count; i++) {
	parm_ptr p = parm_table->item[i];
	if (!strcmp(p->key, key)) return p->value;
    }
    return NULL;
}
