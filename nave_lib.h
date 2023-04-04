#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#ifndef _SEM_LIB_H
	#include "sem_lib.h"
#endif

/*  Genera un punto casuale nella mappa di lato 'lato'.
 * ATTENZIONE: modifica il seed di rand. */
point generate_random_point_nave(int lato);

/* Calcola la distanza tra i due punti di tipo 'point' passati. */
double calcola_distanza(point p1, point p2);

/* Calola il porto più vicino alla posizione p passata come argomento. */
int calcola_porto_piu_vicino(point p, point* ptr_shm_posizioni_porti, int so_porti, int so_lato);

void codice_simulazione(int indice, int PARAMETRO[], int SEM_ID[], int id_coda_richieste,
    void* vptr_shm_dump, void* vptr_shm_posizioni_porti, void* vptr_shm_dettagli_lotti, void* vptr_shm_mercato,
    int *DEB_porti_attraccati, int *DEB_porti_lasciati, int *DEB_porto_ultima_destinazione);

point avvia_nave(int indice, int PARAMETRO[], int SEM_ID[], void* vptr_shm_dump, void* vptr_shm_posizioni_porti, int *DEB_porti_attraccati, int *DEB_porti_lasciati, int *DEB_porto_ultima_destinazione, int *indice_porto_attraccato);

richiesta esamina_porto(int indice, int PARAMETRO[], int SEM_ID[], int id_coda_richieste, void* vptr_shm_posizioni_porti, void* vptr_shm_dettagli_porti, void* vptr_shm_mercato, void* vptr_shm_dettagli_lotti, void* vptr_shm_dump, int* lotti_scartati, int *indice_porto_attraccato, int *reqlett, point posizione, int *spaziolibero, double *tempo_carico, merce_nave carico[], int *i_carico, int *indice_destinazione);

void carica_dal_porto(int indice, int PARAMETRO[], int id_coda_richieste, void *vptr_shm_posizioni_porti, void *vptr_shm_mercato, void *vptr_shm_dettagli_lotti, void *vptr_shm_dump, richiesta r, point posizione, int *indice_destinazione, int *indice_porto_attraccato, int *lotti_scartati, int *spaziolibero, double *tempo_carico, int *i_carico, int *reqlett, merce_nave carico[]);

void attracco_e_scarico(int indice, int PARAMETRO[], int SEM_ID[], int *DEB_porti_attraccati, int *DEB_porto_ultima_destinazione, void *vptr_shm_dump, void * vptr_shm_mercato, int *spaziolibero, int *i_carico, double *tempo_carico, int *reqlett, int *indice_porto_attraccato, merce_nave carico[]);
/* Richiede una banchina al semaforo di indice 'indice_porto', effettuando 
 * una maschera dei segnali per evitare spiacevoli 'loop' dello scheduler.
 * 
 * BLOCCA SIGUSR1.
 * Una volta eseguita la semop(), un eventuale segnale pendente viene consegnato. */
void richiedi_banchina(int id_semaforo_banchine, int indice_porto);

/* Modifica lo stato delle navi nel dump in base all'argomento (int) passato. */
void stato_nave(int stato, int id_semaforo_dump, void *vptr_shm_dump, int indice);

/* Effettua una nanosleep, bloccando SIGUSR1 e ripristinandolo alla fine. 
 * 
 * NOTA: il segnale rimane pendente; in ogni caso la nave aggiorna il proprio stato
 * prima di entrare in attesa, tramite il metodo statoNave(int). */
void attesa(double val, int divisore);

/* Aggiorna il dump sulle merci caricate in nave, anche in riferimento al porto. */
void aggiorna_dump_carico(void *vptr_dump, int indiceporto, merce_nave* carico, int caricati, int spazio_libero, int id_sem_dump, int PARAMETRO[]);

/* Scarica la merce dalla nave aggiornando mercato e dump. */
void scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, void* vptr_mercato, void* vptr_dump, int id_sem_dump, int PARAMETRO[]);