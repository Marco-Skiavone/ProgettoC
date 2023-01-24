#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* The following union must be defined as required by the semctl man
 * page */
union semun {
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array;  /* Array for GETALL, SETALL */
	struct seminfo *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};


/* Crea il semaforo BANCHINE, ritorna il valore di semget */
int crea_semaforo_banchine(int PORTI);

/* esegue solo l'aggancio salvandosi l'id staticamente */
int trova_semaforo_banchine(int PORTI);

/* Crea il semaforo DUMP, ritorna il valore di semget */
int crea_semaforo_dump(int MERCI);

/* esegue solo l'aggancio salvandosi l'id staticamente */
int trova_semaforo_dump(int MERCI);

/* Crea il semaforo GESTIONE, ritorna il valore di semget */
int crea_semaforo_gestione(int PORTI, int NAVI);

/* esegue solo l'aggancio salvandosi l'id staticamente */
int trova_semaforo_gestione(int PORTI, int NAVI);

/* Crea il semaforo MERCATO, ritorna il valore di semget */
int crea_semaforo_mercato(int PORTI);

/* esegue solo l'aggancio salvandosi l'id staticamente */
int trova_semaforo_mercato(int PORTI);

/* distrugge il set MERCATO */
int distruggi_semaforo(int id_semaforo);


/* Setta il semaforo all'indice sem_num col valore sem_val.
 * Ritorna il valore di semctl. */
int sem_set_val(int sem_id, int sem_num, int sem_val);

/* Cerca di accedere (FA --1 BLOCCANTE).
 * Ritorna il valore dell semop. */
int sem_reserve(int sem_id, int sem_num);

/* Rilascia la risorsa: INCREMENTA DI 1:
 * Ritorna il valore della semop. */
int sem_release(int sem_id, int sem_num);

/* Inserisce in *my_string il valore di tutti i semafori del set.
 * my_string deve essere allocato dal chiamante. */
int sem_getall(char * my_string, int sem_id);

/* Setta a tutti i valori di n_sems il valore value */
int sem_setall(int n_sems, int value, int sem_id);