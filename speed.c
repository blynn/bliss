#include <math.h>

double fast_next(double t, double freq)
{
	double ignore;
	return modf(t + freq * (1.0 / 44100.0), &ignore);
}

char low_byte(int i)
{
	return i & 255;
}

char high_byte(int i)
{
	return (i >> 8) & 255;
}
