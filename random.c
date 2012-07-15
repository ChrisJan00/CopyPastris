#include <stdlib.h>

#include "random.h"

int
get_random(int min, int max)
{
	float r;			/* liczba losowa z przedzlalu [0, 1) */

	r = rand() / (float)RAND_MAX;
	return r * abs(max + min) - min;
}
