#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "song.h"

static SDL_AudioSpec wanted;

static song_ptr song;
static int is_playing;

static Uint8 *buf; //circular buffer
static int buf_size;
static int buf_start;
static int buf_empty;
static int buf_end;

static void oldc_fill_audio(void *udata, Uint8 *stream, int len)
{
    int count;
    //TODO: handle odd len
    if (len % 4) {
	printf("len = %d\n", len);
    }
    for (count=0; count<len;) {
	int i, n;
	double l, r;
	if (is_playing) {
	    n = song->machine->count;
	    for (i=0; i<n; i++) ((machine_ptr) song->machine->item[i])->visited = 0;
	    song_next_sample(song, &l, &r);

	    n = l * 32700;
	    if (n > 32767) n = 32767;
	    else if (n < -32767) n = -32767;
	    stream[count] = (Uint8) (n & 255); //lower 16 bits
	    count++;
	    n = n >> 8;
	    stream[count] = (Uint8) (n & 255); //upper 16 bits
	    count++;
	    n = r * 32700;
	    if (n > 32767) n = 32767;
	    else if (n < -32767) n = -32767;
	    stream[count] = (Uint8) (n & 255); //lower 16 bits
	    count++;
	    n = n >> 8;
	    stream[count] = (Uint8) (n & 255); //upper 16 bits
	    count++;
	}
    }
}

static void c_fill_audio(void *udata, Uint8 *stream, int len)
{
    int i;

    if (buf_start == buf_end && buf_empty) {
	//printf("buffer underrun\n");
	return;
    }
    for (i=0; i<len;) {
	stream[i] = buf[buf_start];
	buf_start++;
	i++;
	stream[i] = buf[buf_start];
	buf_start++;
	i++;
	if (buf_start >= buf_size) buf_start = 0;
	if (buf_start == buf_end) {
	    //printf("buffer underrun\n");
	    buf_empty = -1;
	    return;
	}
    }
}

static void append_buf(double x)
{
    int n;
    n = x * 32700;
    if (n > 32767) n = 32767;
    else if (n < -32767) n = -32767;
    buf[buf_end] = (Uint8) (n & 255); //lower 16 bits
    buf_end++;
    n = n >> 8;
    buf[buf_end] = (Uint8) (n & 255); //upper 16 bits
    buf_end++;
    if (buf_end >= buf_size) buf_end = 0;
    buf_empty = 0;
}

static int buffer_ready(void)
{
    return (buf_end != buf_start || buf_empty);
}

void audio_buffer()
{
    while (buffer_ready()) {
	int i, n;
	double l, r;
	if (is_playing) {
	    n = song->machine->count;
	    for (i=0; i<n; i++) ((machine_ptr) song->machine->item[i])->visited = 0;
	    song_next_sample(song, &l, &r);
	    append_buf(l);
	    append_buf(r);
	} else {
	    append_buf(0);
	    append_buf(0);
	}
    }
}

void audio_init()
{
    /* Set the audio format */
    wanted.freq = samprate;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = 1024;  /* Good low-latency value for callback */
    wanted.callback = c_fill_audio;
    wanted.userdata = NULL;

    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
	fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	exit(-1);
    }

    //start with buffer full of zeroes
    buf_size = samprate / 2; //half a sec
    buf = (Uint8 *) malloc(sizeof(Uint8) * buf_size);
    memset(buf, 0, buf_size);
    buf_start = 0;
    buf_end = 0;
    buf_empty = 0;

    is_playing = 0;
    song = NULL;
}

void audio_pause()
{
    is_playing = 0;
}

void audio_play()
{
    is_playing = 1;
}

void audio_put_song(song_ptr s)
{
    song = s;
}
