#ifndef UTIL_H
#define UTIL_H

char *strclone(char *s);
int sample_at(unsigned char *buf, int i);
double note_to_freq(int n);
int notechar_to_int(char c);
int hex_to_int(char c);
char shift_key(unsigned char key);

#endif //UTIL_H
