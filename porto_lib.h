#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#ifndef _QUEUE_LIB_H
	#include "queue_lib.h"
#endif
#ifndef _SEM_LIB_H
	#include "sem_lib.h"
#endif

/* Inizializza la banchina del porto 'indice', aggiornando anche il dump. */
void inizializza_banchine(int sem_id, int indice, void * vptr_shm_dump, int PARAMETRO[]);

/* Genera le merci presso il porto di indice 'indice'. Infine carica sul dump 
 * lo stato del mercato del porto in questione. 
 * 
 * NB: 'Fill' corrisponde alla quantità (in ton) di merce da suddividere tra richiesta e offerta per ogni porto. */
void spawnMerciPorti(void* vptr_mercato, merce* ptr_lotti, void *vptr_dump, int id_sem_dump, int PARAMETRO[], int indice);

/* Invia le richieste alla coda messaggi in base ai valori di mercato_shm. */
void manda_richieste(void* vptr_shm_mercato, int indice, int coda_id, int PARAMETRO[]);