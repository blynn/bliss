#include <stdio.h>

void base64_encode(FILE *fp, unsigned char *data, int len);
void base64_decode(unsigned char **data, int *len, FILE *fp);
