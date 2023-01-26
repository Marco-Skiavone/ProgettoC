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

/*funzione aggiornamento dump merce:
 * stato = 0: presente in porto (D_M_PRES_PORTO)
 * stato = 1: presente in nave (D_M_PRES_NAVE)
 * stato = 2: consegnata (D_M_CONSEGNATA)
 * stato = 3: scaduta in porto (D_M_SCADUTA_PORTO)
 * stato = 4: scaduta in nave (D_M_SCADUTA_NAVE) */
void aggiorna_dump_merce(dump *ptr_dump, int id_sem_dump, int indice_merce, int stato);

/* stato = 0: nave in porto (D_N_PORTO)
 * stato = 1: nave in mare con carico (D_N_CARICA)
 * stato = 2: nave in mare senza carico (D_N_SCARICA) */
void aggiorna_dump_nave(dump *ptr_dump, int id_sem_dump, int MERCI, int stato);