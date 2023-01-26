#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* Esegue le nanosleep di spostamento, mascherando i segnali
 e settando lo stato sul dump, finita la nanosleep aggiorna la posizione */
void spostamento(viaggio v, point *p);

/* ritorna il valore della nanosleep, 
 * "divisore" definisce il divisore per trovare i secondi */
int attesa(double to_wait, int divisore);

double calcola_distanza(double x1, double y1, double x2, double y2 );

int calcola_porto_piu_vicino(point p, point *ptr_shm_porti, int par_SO_PORTI);

/* restituisce un viaggio verso porto generico */
viaggio porto_generico(int i_porto, point p_n, int SZ_POSIZIONI, int SPEED, point *ptr_posizioni);

/*restituisce struct con info per il porto più vicino alla nave*/
viaggio porto_piu_vicino(point p_n, int SZ_POSIZIONI, int PORTI, int SPEED, point *ptr_posizioni);

/* se stato = 0: nave in porto */
void aggiorna_dump_nave(int indice_nave, int stato);