#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "my_lib.h"

/* genera e restituisce un punto in maniera randomica, dato SO_LATO */
point generate_rand_point(int LATO){
	/*valori ausiliari generazione punto casuale*/
	int mant, p_intera;
	point p;
	/*coordinata x*/
	p_intera = mant = rand()%LATO;
	/*parte decimale + parte intera*/
	p.x = ((double)mant/LATO) + (p_intera*getppid()%LATO);
	/*coordinata y*/
	/*srand(mant);*/
	p_intera = mant = rand()%LATO;
	p.y = ((double)mant/LATO) + ((p_intera*getppid()%LATO));
	return p;
}

/* Metodo per vedere se 2 double sono accettabilmente distanti (TOLLERANZA) s*/
int equals(double x, double y){
	if(x > y)
		return (x-y) < TOLLERANZA ? 1: 0;
	else 
		return (y-x) < TOLLERANZA ? 1 : 0;
}