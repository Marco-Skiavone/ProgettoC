#include "definitions.h"
#include "my_lib.h"

/* Set a semaphore to a user defined value */
int sem_set_val(int sem_id, int sem_num, int sem_val) {
	return semctl(sem_id, sem_num, SETVAL, sem_val);
}

/* Try to access the resource */
int sem_reserve(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* Release the resource */
int sem_release(int sem_id, int sem_num) {
	struct sembuf sops;
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* Print all semaphore values to a string */
int sem_getall(char * my_string, int sem_id) {
	union semun arg;   /* man semctl per vedere def della union  */ 
	unsigned short * sem_vals, i;
	unsigned long num_sem;
	char cur_str[10];
	
	/* Get the number of semaphores */
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
}

/* Setta tutti i n_sems semafori ai valori */
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

point generate_rand_point(int LATO){
	/*valori ausiliari generazione punto casuale*/
	int p_intera, mantissa;
	point p;
	/*coordinata x*/
	srand(SEED);
	mantissa = p_intera = rand()%LATO;
	/*parte decimale + parte intera*/
	p.x = ((double)p_intera/LATO) + (mantissa*getppid()%LATO);
	/*coordinata y*/
	srand(p_intera);
	mantissa = p_intera = rand()%LATO;
	p.y = ((double)p_intera/LATO) + ((mantissa*getppid()%LATO));

}