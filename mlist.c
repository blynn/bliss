#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pl.h"
#include "mlist.h"
#include "util.h"
#include "master.h"
#include "plugin.h"
#include "bmachine.h"

darray_t mlist;
darray_t ulist;

machine_info_ptr master_mi;

static int right_form(char *filename)
{
    int i;

    i = strlen(filename);
    i -= 3;
    if (strcmp(&filename[i], ".so")) return 0;
    return 1;
}

static void try_load(char *filename)
{
    void *p;
    int (*typefunc)();
    void (*func)(machine_info_ptr);
    void (*ufunc)(unit_info_ptr);
    machine_info_ptr mi;
    unit_info_ptr ui;

    p = pl_load(filename);
    if (!p) return;

    typefunc = pl_sym(p, "bliss_plugin_type");
    if (!typefunc) {
	printf("%s: can't get symbol bliss_plugin_type\n", filename);
	pl_clear(p);
	return;
    }

    switch(typefunc()) {
	case unit_plugin:
	    ufunc = pl_sym(p, "unit_info_init");
	    if (!ufunc) {
		//plugin is missing symbol
		printf("plugin is missing symbol!\n");
		pl_clear(p);
		return;
	    }
	    ui = (unit_info_ptr) malloc(sizeof(unit_info_t));
	    ufunc(ui);
	    ui->dlptr = p;
	    ui->plugin = strclone(filename);
	    darray_append(ulist, ui);
	    break;
	case machine_plugin:
	    func = pl_sym(p, "machine_info_init");
	    if (!func) {
		//plugin is missing symbol
		printf("plugin is missing symbol!\n");
		pl_clear(p);
		return;
	    }

	    //printf("loaded '%s': %s\n", filename, mi->id);
	    mi = machine_info_new();
	    func(mi);
	    mi->dlptr = p;
	    mi->plugin = strclone(filename);
	    darray_append(mlist, mi);
	    break;
	default:
	    pl_clear(p);
	    break;
    }
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

unit_info_ptr unit_info_at(char *id)
{
    int i, n;

    n = ulist->count;
    for (i=0; i<n; i++) {
	unit_info_ptr mi = (unit_info_ptr) ulist->item[i];
	if (!strcmp(mi->id, id)) return mi;
    }
    return NULL;
}

void mlist_init()
{
    master_mi = machine_info_new();
    master_machine_info_init(master_mi);
    darray_init(mlist);
    darray_init(ulist);
    darray_append(mlist, master_mi);
}

void mlist_clear()
{
    //TODO
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

machine_info_ptr new_bmachine(char *id, char *name)
{
    machine_info_ptr mi = machine_info_new();
    bmachine_info_init(mi);
    mi->type = machine_bliss;
    mi->id = strclone(id);
    mi->name = strclone(name);
    darray_append(mlist, mi);
    return mi;
}
