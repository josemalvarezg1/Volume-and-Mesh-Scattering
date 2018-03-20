#include "Halton.h"

double halton_sequence(int index, int base)
{
	double f, r;
	f = 1.0;
	r = 0.0;
	while (index > 0)
	{
		f = f / base;
		r = r + f * (index % base);
		index = index / base;
	}
	return r;
}