#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

void base64_encode(FILE *fp, unsigned char *data, int len)
{
    static int first = 1;
    static char table[64];
    int imod3 = 0;
    int i, n = 0;
    int col = 0;

    void out(char c) {
	fputc(c, fp);
	col++;
	if (col == 70) {
	    col = 0;
	    fputc('\n', fp);
	}
    }
    char c;
    if (first) {
	first = 0;
	i = 0;
	for (c='A';c<='Z'; c++) {
	    table[i] = c;
	    i++;
	}
	for (c='a';c<='z'; c++) {
	    table[i] = c;
	    i++;
	}
	for (c='0';c<='9'; c++) {
	    table[i] = c;
	    i++;
	}
	table[i] = '+';
	i++;
	table[i] = '/';
    }
 
    for (i=0; i<len; i++) {
	switch(imod3) {
	    case 0:
		n = data[i] >> 2;
		out(table[n]);
		n = data[i] % (1 << 2);
		break;
	    case 1:
		n = (n << 4) + (data[i] >> 4);
		out(table[n]);
		n = data[i] % (1 << 4);
		break;
	    case 2:
		n = (n << 2) + (data[i] >> 6);
		out(table[n]);
		n = data[i] % (1 << 6);
		out(table[n]);
		break;
	    default:
		break;
	}
	imod3++;
	if (imod3 == 3) imod3 = 0;
    }
    //don't care about spillage over 70 columns now:
    col = 0;
    switch(imod3) {
	case 1:
	    n = (n << 4);
	    out(table[n]);
	    out('=');
	    out('=');
	    break;
	case 2:
	    n = (n << 2);
	    out(table[n]);
	    out('=');
	default:
	    break;
    }
    fprintf(fp, "\n====\n");
}

void base64_decode(unsigned char **data, int *len, FILE *fp)
{
    int buf_max = 1024;
    unsigned char *buf = (unsigned char *) malloc(buf_max);
    int i, j;
    unsigned char c;
    static int first = 1;
    static int table[256];
    int n = 0;

    void add_buf(int n)
    {
	buf[i++] = n;
	if (i >= buf_max) {
	    buf_max *= 2;
	    buf = (unsigned char *) realloc(buf, buf_max);
	}
    }

    int cmp(char *s)
    {
	char buf[8];
	fread(buf, 1, strlen(s), fp);
	return strncmp(buf, s, strlen(s));
    }

    if (first) {
	first = 0;
	for (i=0; i<256; i++) table[i] = -1;
	i = 0;
	for (c='A'; c<='Z'; c++) {
	    table[c] = i++;
	}
	for (c='a'; c<='z'; c++) {
	    table[c] = i++;
	}
	for (c='0'; c<='9'; c++) {
	    table[c] = i++;
	}
	c = '+';
	table[c] = i++;
	c = '/';
	table[c] = i++;
    }

    i = 0;
    j = 0;
    for (;;) {
	do {
	    c = fgetc(fp);
	} while(c == '\n');

	if (c == '=') {
	    break;
	} else if (table[c] < 0) {
	    goto err;
	} else {
	    switch (j) {
		case 0:
		    n = table[c] << 2;
		    break;
		case 1:
		    n += table[c] >> 4;
		    add_buf(n);
		    n = table[c] << 4;
		    break;
		case 2:
		    n += table[c] >> 2;
		    add_buf(n);
		    n = table[c] % (1 << 2);
		    n <<= 6;
		    break;
		case 3:
		    n += table[c];
		    add_buf(n);
		    break;
	    }
	    j = (j + 1) % 4;
	}
    }

    switch (j) {
	case 0:
	    if (cmp("===\n")) goto err;
	    break;
	case 1:
	    goto err;
	case 2:
	    if (cmp("=\n====\n")) goto err;
	    break;
	case 3:
	    if (cmp("\n====\n")) goto err;
	    break;
    }

    *data = buf;
    *len = i;
    return;

err:
    free(buf);
    *data = NULL;
    *len = 0;
    return;
}

/*
main() {
    int i;
    unsigned char *data; int len;
    base64_decode(&data, &len, stdin);
    for (i=0; i<len; i++) printf("%c", data[i]);
    printf("\n");
}
*/
