#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "sem_lib.h"


/* Crea il semaforo BANCHINE, ritorna il valore di semget */
int crea_semaforo_banchine(int PORTI){
	int id_semaforo_banchine = semget(KEY_SEM_BANCHINE, PORTI, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return id_semaforo_banchine;
}

/* esegue solo l'aggancio salvandosi l'id staticamente */
int trova_semaforo_banchine(int PORTI){
	int id_semaforo_banchine = semget(KEY_SEM_BANCHINE, PORTI, PERMESSI);
	TEST_ERROR
	return id_semaforo_banchine;
}


/* Crea il semaforo DUMP, ritorna il valore di semget */
int crea_semaforo_dump(int MERCI){
	int id_semaforo_dump = semget(KEY_SEM_DUMP, MERCI+1, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return id_semaforo_dump;
}

/* esegue solo l'aggancionave salvandosi l'id staticamente */
int trova_semaforo_dump(int MERCI){
	int id_semaforo_dump = semget(KEY_SEM_DUMP, MERCI+1, PERMESSI);
	TEST_ERROR
	return id_semaforo_dump;
}

/* Crea il semaforo GESTIONE, ritorna il valore di semget */
int crea_semaforo_gestione(){
	int id_semaforo_gestione = semget(KEY_SEM_GESTIONE, 1, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return id_semaforo_gestione;
}
int trova_semaforo_gestione(){
	int id_semaforo_gestione = semget(KEY_SEM_GESTIONE, 1, PERMESSI);
	TEST_ERROR
	return id_semaforo_gestione;
}

/* Crea il semaforo MERCATO, ritorna il valore di semget */
int crea_semaforo_mercato(int PORTI){
	int id_semaforo_mercato = semget(KEY_SEM_MERCATO, PORTI, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	return id_semaforo_mercato;
}

/* esegue solo l'aggancio salvandosi l'id staticamente */
int trova_semaforo_mercato(int PORTI){
	int id_semaforo_mercato = semget(KEY_SEM_MERCATO, PORTI, PERMESSI);
	TEST_ERROR
	return id_semaforo_mercato;
}

/* distrugge il set MERCATO */
int distruggi_semaforo(int id_semaforo){
	int ctl_val = semctl(id_semaforo, 0, IPC_RMID);
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

int sem_waitforzero(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = 0;
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
int sem_setall(int sem_id, int n_sems, unsigned short value) {
	union semun arg;   /* man semctl per vedere def della union  */ 
	unsigned short buf_valori[n_sems];
	unsigned short *sem_vals, i;

	/* Set all values to value */
	for(i = 0; i < n_sems; i++)
		buf_valori[i] = value;
	/* Get the values of all semaphores */
	arg.array = buf_valori;
	semctl(sem_id, 0, SETALL, arg);
	TEST_ERROR
	return errno;
}