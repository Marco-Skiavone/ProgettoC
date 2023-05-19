#ifndef _PORTO_LIB_H
#define _PORTO_LIB_H
#include "definitions.h"
#include "sem_lib.h"
#include "queue_lib.h"

/* Inizializza la banchina del porto 'indice', aggiornando anche il dump. */
void inizializza_banchine(int sem_id, int indice, void * vptr_shm_dump, int PARAMETRO[]);

/* Genera le merci presso il porto di indice 'indice'. Infine carica sul dump 
 * lo stato del mercato del porto in questione. 
 * 
 * NB: 'Fill' corrisponde alla quantità (in ton) di merce da suddividere tra richiesta e offerta per ogni porto. */
void spawnMerciPorti(void* vptr_mercato, merce* ptr_lotti, void *vptr_dump, int id_sem_dump, int id_sem_merci, int PARAMETRO[], int indice, int coda_id, int fd_fifo);

/** Invia le richieste alla coda messaggi in base ai valori di mercato_shm.
 * @param fd_fifo è il file descriptor della FIFO in cui mandare le richieste che eccedono il limite della coda. */
void manda_richieste(void* vptr_shm_mercato, int indice, int coda_id, int PARAMETRO[], int fd_fifo);
#endif