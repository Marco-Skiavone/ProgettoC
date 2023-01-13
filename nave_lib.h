#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* Esegue le nanosleep di spostamento, mascherando i segnali
 e settando lo stato sul dump, finita la nanosleep aggiorna la posizione */
void spostamento(viaggio v, point *p);

/* Funzione spostamento nave*/
double distanza (double x, double y, point *p);

/*viaggio verso porto generico*/
viaggio porto_generico(int i_porto, point *p_n, int SZ_POSIZIONI, int SPEED);

/*restituisce struct con info per il porto più vicino alla nave*/
viaggio porto_piu_vicino(point *p_n, int SZ_POSIZIONI, int PORTI, int SPEED);