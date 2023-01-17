#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "sem_lib.h"

/* id del set di semafori BANCHINE */
static int banchine_id;
/* id del set di semafori DUMP */
static int dump_id;
/* id del set di semafori GESTIONE */
static int gestione_id;
/* id del set di semafori MERCATO */
static int mercato_id;

/* Crea il semaforo BANCHINE, ritorna il valore di semget */
int crea_semaforo_anchine(int PORTI, int BANCHINE){
	banchine_id = semget(KEY_SEM_BANCHINE, PORTI, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return banchine_id;
}

/* distrugge il set BANCHINE */
int distruggi_semaforo_banchine(){
	int ctl_val = semctl(banchine_id, 0, IPC_RMID);
	TEST_ERROR
	return ctl_val;
}

/* Crea il semaforo DUMP, ritorna il valore di semget */
int crea_semaforo_dump(int MERCI){
	dump_id = semget(KEY_SEM_DUMP, MERCI+3, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return dump_id;
}

/* distrugge il set DUMP */
int distruggi_semaforo_dump(){
	int ctl_val = semctl(dump_id, 0, IPC_RMID);
	TEST_ERROR
	return ctl_val;
}

/* Crea il semaforo GESTIONE, ritorna il valore di semget */
int crea_semaforo_gestione(int MERCI){
	gestione_id = semget(KEY_SEM_GESTIONE, MERCI+3, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return gestione_id;
}

/* distrugge il set GESTIONE */
int distruggi_semaforo_gestione(){
	int ctl_val = semctl(gestione_id, 0, IPC_RMID);
	TEST_ERROR
	return ctl_val;
}

/* Crea il semaforo MERCATO, ritorna il valore di semget */
int crea_semaforo_mercato(int PORTI){
	gestione_id = semget(KEY_SEM_MERCATO, PORTI, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return gestione_id;
}

/* distrugge il set MERCATO */
int distruggi_semaforo_mercato(){
	int ctl_val = semctl(gestione_id, 0, IPC_RMID);
	TEST_ERROR
	return ctl_val;
}

/* Setta il semaforo all'indice sem_num col valore sem_val.
 * Ritorna il valore di semctl. */
int sem_set_val(int sem_id, int sem_num, int sem_val) {
	return semctl(sem_id, sem_num, SETVAL, sem_val);
}

/* Cerca di accedere (FA --1 BLOCCANTE).
 * Ritorna il valore dell semop. */
int sem_reserve(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* Rilascia la risorsa: INCREMENTA DI 1:
 * Ritorna il valore della semop. */
int sem_release(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* Inserisce in *my_string il valore di tutti i semafori del set.
 * my_string deve essere allocato dal chiamante. */
int sem_getall(char * my_string, int sem_id) {
	union semun arg;
	unsigned short *sem_vals, i;
	unsigned long num_sem;
	char cur_str[10];
	
	semctl(sem_id, 0, IPC_STAT, arg.buf);
	TEST_ERROR;
	num_sem = arg.buf->sem_nsems;
	
	/* Get the values of all semaphores */
	sem_vals = malloc(sizeof(*sem_vals)*num_sem);
	arg.array = sem_vals;
	semctl(sem_id, 0, GETALL, arg);
	
	/* Initialize the string. MUST be allocated by the caller */
	my_string[0] = 0;
	for (i=0; i<num_sem; i++) {
		sprintf(cur_str, "%d ", sem_vals[i]);
		strcat(my_string, cur_str);
	}
	TEST_ERROR
	return errno;
}

/* Setta a tutti i valori di n_sems il valore value */
int sem_setall(int n_sems, int value, int sem_id) {
	union semun arg;   /* man semctl per vedere def della union  */ 
	short unsigned int *buf_valori = (short unsigned int *) malloc(n_sems*sizeof(int));
	int i;

	/* Set all values to value */
	for(i = 0; i < n_sems; i++)
		buf_valori[i] = value;
	
	/* Get the values of all semaphores */
	arg.array = buf_valori;
	semctl(sem_id, 0, SETALL, arg);
	TEST_ERROR
	return errno;
}