#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pl.h"
#include "mlist.h"
#include "util.h"
#include "master.h"

darray_t mlist;

machine_info_t master_mi;

static int right_form(char *filename)
{
    int i;

    i = strlen(filename);
    i -= 3;
    if (strcmp(&filename[i], ".so")) return 0;
    return 1;
}

//default callbacks: do nothing
static void nop_work(machine_t m, double *l, double *r)
{
}

static void nop_init(machine_t m)
{
}

static void nop_clear(machine_t m)
{
}

static void nop_parse(machine_t m, cell_t c, int col)
{
}

static void nop_print_state(machine_t m, FILE *fp)
{
}

static void nop_tick(machine_t m)
{
}

static void text_cell_init(cell_t c, machine_t m, char *text, int col)
{
    cell_init_string(c, text);
}

void machine_info_assign_default(machine_info_ptr mi)
{
    mi->init = nop_init;
    mi->clear = nop_clear;
    mi->work = nop_work;
    mi->parse = nop_parse;
    mi->tick = nop_tick;
    mi->print_state = nop_print_state;
    mi->cell_init = text_cell_init;
    mi->buzzmi = NULL;
}

static void try_load(char *filename)
{
    void *p;
    void (*func)(machine_info_ptr);
    machine_info_ptr mi;

    p = pl_load(filename);
    if (!p) return;

    func = pl_sym(p, "machine_info_init");
    if (!func) {
	//plugin is missing symbol
	printf("plugin is missing symbol!\n");
	pl_clear(p);
	return;
    }

    //printf("loaded '%s': %s\n", filename, mi->id);
    mi = (machine_info_ptr) malloc(sizeof(machine_info_t));
    machine_info_assign_default(mi);
    func(mi);
    mi->dlptr = p;
    mi->plugin = strclone(filename);
    darray_append(mlist, mi);
}

machine_info_ptr machine_info_at(char *id)
{
    int i, n;

    n = mlist->count;
    for (i=0; i<n; i++) {
	machine_info_ptr mi = (machine_info_ptr) mlist->item[i];
	if (!strcmp(mi->id, id)) return mi;
    }
    return NULL;
}

void mlist_init()
{
    machine_info_assign_default(master_mi);
    master_machine_info_init(master_mi);
    darray_init(mlist);
    darray_append(mlist, master_mi);
}

void load_plugin_dir(char *dirname)
{
    //sort list alphabetically
    int micmp(const void *e1, const void *e2) {
	machine_info_ptr m1 = *((machine_info_ptr *) e1);
	machine_info_ptr m2 = *((machine_info_ptr *) e2);
	return strcmp(m1->id, m2->id);
    }
    DIR *dp;
    struct dirent *de;
    char *s;

    dp = opendir(dirname);

    if (!dp) return;

    for (;;) {
	de = readdir(dp);
	if (!de) break;
	if (right_form(de->d_name)) {
	    s = (char *) malloc(strlen(de->d_name) + strlen(dirname) + 2);
	    strcpy(s, dirname);
	    strcat(s, "/");
	    strcat(s, de->d_name);
	    try_load(s);
	    free(s);
	}
    }

    closedir(dp);

    qsort(mlist->item, mlist->count, sizeof(machine_info_ptr), micmp);
}
