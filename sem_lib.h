#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* Crea un set di semafori date la chiave e il numero di semafori. */
int sem_create(key_t key, int nsems);

/* Si aggancia a un set di semafori già esistente. */
int sem_find(key_t key, int nsems);

/* Esegue la reserve, ossia si blocca finché non può fare -1 al semaforo. */
void sem_reserve(int semid, int sem_num);

/* Esegue la release, ossia fa +1 al semaforo. */
void sem_release(int semid, int sem_num);

/* Esegue la 'waitforzero'. */
void sem_wait_zero(int semid, int sem_num);

/* Setta il valore del semaforo nella posizione data con 'val'. */
void sem_set_val(int semid, int sem_num, int val);

/* Distrugge il semaforo. */
void sem_destroy(int semid);

/* Restituisce il valore del semaforo nella posizione data. */
int sem_get_val(int sem_id, int sem_num);

/* Inizializza gli id dei semafori. */
void inizializza_semafori(int *id_mercato, int *id_gestione, int *id_banchine, int *id_dump, int PORTI);