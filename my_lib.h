#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* genera e restituisce un punto in maniera randomica, dato SO_LATO */
point generate_rand_point(int LATO);

/* metodo per vedere se 2 double sono accettabilmente distanti (TOLLERANZA) s*/
int equals(double x, double y);