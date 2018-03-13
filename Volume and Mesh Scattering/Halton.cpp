#include "Halton.h"

double Halton_Seq(int index, int base) {
	double f = 1, r = 0;
	while (index > 0) {
		f = f / base;
		r = r + f* (index% base);
		index = index / base;
	}
	return r;
}