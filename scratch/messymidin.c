#include <sys/time.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "darray.h"

enum {
    t_note_on,
    t_note_off,
    t_controller,
};

struct midi_event_s {
    int sec;
    int usec;
    int type;
    int arg1, arg2;
    unsigned char s[8];
    int len;
};
typedef struct midi_event_s *midi_event_ptr;
typedef struct midi_event_s midi_event_t[1];

int fd;
unsigned char buf[80];
int tick;
darray_t mtrk;

static struct timeval tv_start;
static struct timeval tv_stamp;

static pthread_t control_thread;

void add_event(int type, int a1, int a2)
{
    int ms;
    int i;
    midi_event_ptr e = (midi_event_ptr) malloc(sizeof(midi_event_t));
    midi_event_ptr e0;
    e->type = type;
    e->sec = tv_stamp.tv_sec;
    e->usec = tv_stamp.tv_usec;
    e->arg1 = a1;
    e->arg2 = a2;
    ms = e->sec * 1000 + (e->usec / 1000);
    if (mtrk->count) {
	e0 = (midi_event_ptr) mtrk->item[mtrk->count - 1];
	ms -= e0->sec * 1000 + (e0->usec / 1000);
    }

    e->len = 0;

    for (i=3; i>0; i--) {
	if ((e->s[e->len] = (ms >> (7 * i)) & 0x7F)) {
	    e->s[e->len] |= 0x80;
	    e->len++;
	}
    }
    e->s[e->len] = ms & 0x7F;
    e->len++;
    switch(type) {
	case t_note_on:
	    e->s[e->len] = 0x90;
	    e->len++;
	    e->s[e->len] = e->arg1;
	    e->len++;
	    e->s[e->len] = e->arg2;
	    e->len++;
	    break;
	case t_note_off:
	    e->s[e->len] = 0x80;
	    e->len++;
	    e->s[e->len] = e->arg1;
	    e->len++;
	    e->s[e->len] = e->arg2;
	    e->len++;
	    break;
	case t_controller:
	    e->s[e->len] = 0xB0;
	    e->len++;
	    e->s[e->len] = e->arg1;
	    e->len++;
	    e->s[e->len] = e->arg2;
	    e->len++;
	    break;
    }
    darray_append(mtrk, e);
}

void note_on(int n)
{
    read(fd, buf, 2);
    add_event(t_note_on, buf[0], buf[1]);
}

void note_off(int n)
{
    read(fd, buf, 2);
    add_event(t_note_off, buf[0], buf[1]);
}

void controller(int n)
{
    read(fd, buf, 2);
    add_event(t_controller, buf[0], buf[1]);
}

void patch(int n)
{
    read(fd, buf, 1);
}

void timer_start()
{
    gettimeofday(&tv_start, NULL);
}

void timer_stamp()
{
    gettimeofday(&tv_stamp, NULL);
    tv_stamp.tv_usec -= tv_start.tv_usec;
    tv_stamp.tv_sec -= tv_start.tv_sec;
    if (tv_stamp.tv_usec < 0) {
	tv_stamp.tv_sec--;
	tv_stamp.tv_usec+=1e6;
    }
}

static int sysex;

void handle_f(int n)
{
    switch (n) {
	case 0x0:
	    //sysex
	    read(fd, buf, 1);
	    fprintf(stderr, "%d: Mfr ID = %d\n", tick, buf[0]);
	    sysex = 1;
	    break;
	case 0x7:
	    //end sysex
	    fprintf(stderr, "%d: end sysex\n", tick);
	    break;
	case 0x8:
	    //clock
	    //if (!(tick % 24)) printf("tick!\n");
	    tick++;
	    break;
	case 0xE:
	    //active sense
	    break;
	default:
	    fprintf(stderr, "unhandled f: %d\n", n);
	    break;
    }
}

void status_byte(unsigned char c)
{
    int type = c >> 4;
    int n = c % (1 << 4);
    timer_stamp();
    switch(type) {
	case 0x8:
	    note_off(n);
	    break;
	case 0x9:
	    note_on(n);
	    break;
	case 0xB:
	    controller(n);
	    break;
	case 0xC:
	    patch(n);
	    break;
	case 0xF:
	    handle_f(n);
	    break;
	default:
	    fprintf(stderr, "unhandled: %d %d\n", type, n);
	    break;
    }
}

int stopflag = 0;
static void *control_loop()
{
    char c;
    c = fgetc(stdin);
    stopflag = 1;
    return NULL;
}

void print_mtrk()
{
    int i, n;
    n = mtrk->count;
    int len;
    //magic string + header length
    printf("MThd%c%c%c%c", 0, 0, 0, 6);
    //format type
    printf("%c%c", 0, 0);
    //number of tracks
    printf("%c%c", 0, 1);
    //divisions
    //printf("%c%c", -25, 40);
    printf("%c%c", 500 >> 8, 500 & 0xff);

    len = 0;
    for (i=0; i<n; i++) {
	midi_event_ptr e = (midi_event_ptr) mtrk->item[i];
	len += e->len;
    }

    len += 4;

    printf("MTrk");
    printf("%c%c%c%c", len >> 24, (len >> 16) & 0xFF, (len >> 8) & 0xFF, len & 0xFF);

    for (i=0; i<n; i++) {
	midi_event_ptr e = (midi_event_ptr) mtrk->item[i];
	int j;
	//fprintf(stderr, "%d.%06d event\n", e->sec, e->usec);
	for (j=0; j<e->len; j++) {
	    printf("%c", e->s[j]);
	}
    }
    printf("%c%c%c%c", 0, 0xFF, 0x2F, 0);

}

int main()
{

    fd = open("/dev/midi00", O_RDONLY);

    darray_init(mtrk);
    pthread_create(&control_thread, NULL, control_loop, NULL);
    timer_start();
    while (!stopflag) {
	read(fd, buf, 1);
	if (buf[0] & 0x80) {
	    sysex = 0;
	    status_byte(buf[0]);
	} else {
	    if (sysex) {
		fprintf(stderr, "sysex data: %x\n", buf[0]);
	    } else {
		fprintf(stderr, "nonstatus: %x\n", buf[0]);
	    }
	}
    }

    print_mtrk();
    return 0;
}
