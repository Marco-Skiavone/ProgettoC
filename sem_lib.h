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
int creaBanchine(int PORTI, int BANCHINE){
	banchine_id = semget(KEY_SEM_BANCHINE, PORTI, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return banchine_id;
}

/* distrugge il set BANCHINE */
int distruggiBanchine();

/* Crea il semaforo DUMP, ritorna il valore di semget */
int creaDump(int MERCI);

/* distrugge il set DUMP */
int distruggiDump();

/* Crea il semaforo GESTIONE, ritorna il valore di semget */
int creaGestione(int MERCI);

/* distrugge il set GESTIONE */
int distruggiGestione();

/* Crea il semaforo MERCATO, ritorna il valore di semget */
int creaMercato(int PORTI);

/* distrugge il set MERCATO */
int distruggiMercato();

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