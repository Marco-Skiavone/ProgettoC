#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* genera e restituisce un punto in maniera randomica, dato SO_LATO */
point generate_rand_point(int LATO);

/* metodo per vedere se 2 double sono accettabilmente distanti (TOLLERANZA) s*/
int equals(double x, double y);

/*controlla se la coda delle richieste è terminata*/
int fine_richieste(int id);

/*ritorna 0 se esistono ancora offerte, 1 altrimenti */
int fine_offerte(merce *ptr_mercato, int PORTI, int MERCI, int data);

void alarm_handler(int signal);