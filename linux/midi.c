#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "midi.h"

static int fd;
static unsigned char buf[80];

static pthread_t read_thread;

static midi_cb_ptr cbp;

static void midi_note_on(int n)
{
    read(fd, buf, 2);
    //printf("note on: %d %d\n", buf[0], buf[1]);

    if (buf[1]) {
	cbp->note_on(buf[0], buf[1]);
    } else {
	cbp->note_off(buf[0]);
    }
}

static void midi_note_off(int n)
{
    read(fd, buf, 2);
    cbp->note_off(buf[0]);
}

static void controller(int n)
{
    read(fd, buf, 2);
    //printf("cont: %d %d\n", buf[0], buf[1]);
}

static void patch(int n)
{
    read(fd, buf, 1);
}

static int sysex;

static void handle_f(int n)
{
    switch (n) {
	case 0x0:
	    //sysex
	    read(fd, buf, 1);
	    sysex = 1;
	    break;
	case 0x7:
	    //end sysex
	    break;
	case 0x8:
	    break;
	case 0xE:
	    //active sense
	    break;
	default:
	    fprintf(stderr, "unhandled f: %d\n", n);
	    break;
    }
}

static void status_byte(unsigned char c)
{
    int type = c >> 4;
    int n = c % (1 << 4);
    switch(type) {
	case 0x8:
	    midi_note_off(n);
	    break;
	case 0x9:
	    midi_note_on(n);
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

static int stopflag = 0;

static void *read_loop(void *data)
{
    fd = open("/dev/midi00", O_RDONLY);

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

    close(fd);
    return NULL;
}

int midi_start(midi_cb_ptr p)
{
    cbp = p;
    pthread_create(&read_thread, NULL, read_loop, NULL);

    return 0;
}

void midi_stop()
{
    stopflag = 1;
}
