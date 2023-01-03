#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "my_sem_lib.h"
#include "definitions.h"

#define ERROR(str)											\
	fprintf(stderr, "\nErrore %s a linea %d!\n", str, __LINE__);

#define TEST_ERROR 							\
	if (errno) {							\
		fprintf(stderr,						\
		"%s:%d: PID=%5d: Error %d (%s)\n", 	\
		__FILE__,							\
		__LINE__,							\
		getpid(),							\
		errno,								\
		strerror(errno));					\
	}

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];


int main(int argc, char *argv[]){
	int NUM_RIGA_FILE, i, file_config_char;
	int dump_id, porti_id, sem_banchine_id;
	int childs[PARAMETRO[SO_NAVI]+PARAMETRO[SO_PORTI]];
	FILE *file_config;
	char *str = (char *) malloc(MAX_FILE_STR_LEN);
	char *dump_p = (char *) malloc(SIZE_DUMP);
	char *porti_p = (char *) malloc(SIZE_MEM_PORTI);
	char *argv_porti[QNT_PARAMETRI+2], *argv_navi[QNT_PARAMETRI+1]; /* null terminated */
	/* fine definizioni di tipi */
	if(argc != 2){
		fprintf(stderr, "Ri-eseguire con il parametro: var=[NUM_RIGA_FILE]\n");
		exit(EXIT_FAILURE);
	}
	NUM_RIGA_FILE = atoi(argv[1]);
	if(NUM_RIGA_FILE < 1){
		ERROR("parametro inserito in entrata (%d) non adatto")
		fprintf(stderr, "NUM_RIGA_FILE: %d\n", NUM_RIGA_FILE);
		exit(EXIT_FAILURE);
	}

	file_config = fopen("config.txt", "r");
	TEST_ERROR
	
	/* mangia -con fgetc()- tutti i caratteri fino a quello da cui iniziare */
	for(i = 0; i < NUM_RIGA_FILE;){
		if(file_config_char = fgetc(file_config) == '\n')
			i++;
		else if(file_config_char == EOF){
			ERROR("nella ricerca dell'indice passato come parametro")
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}
	
	for(i = 0; i < QNT_PARAMETRI; i++){
		if(fscanf(file_config, "%d", &PARAMETRO[i]) != 1){
			ERROR("nella lettura di un parametro da file")
			fprintf(stderr, "Indice dell'errore su file: %d\n", i);
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}

	printf("Parametri:\nSO_NAVI | SO_PORTI | SO_MERCI | SO_SIZE | SO_MIN_VITA | SO_MAX_VITA | ");
	printf("SO_LATO | SO_SPEED | SO_CAPACITY | SO_BANCHINE | SO_FILL | SO_LOADSPEED | SO_DAYS\n");
	for(i = 0; i < QNT_PARAMETRI; i++){
		printf(" %d ", PARAMETRO[i]);
		if(i != QNT_PARAMETRI-1)
			printf("|");
		else printf("\n");
	}
	
	/* creazione della shm di dump */
	dump_id = shmget(DUMP_KEY, SIZE_DUMP, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	dump_p = shmat(dump_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Dump creato con id = %d\n", dump_id);
	#endif

	/* creazione della shm "registro dei porti" */
	porti_id = shmget(PORTI_MEM_KEY, SIZE_MEM_PORTI, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	porti_p = shmat(dump_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Registro porti creato con id = %d\n", porti_id);
	#endif

	/* creazione del set di semafori "banchine dei porti" */
	sem_banchine_id = semget(BANCHINE_SEM_KEY, PARAMETRO[SO_PORTI], IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	sem_setall(PARAMETRO[SO_PORTI], PARAMETRO[SO_BANCHINE], sem_banchine_id);
	#ifdef DEBUG
		printf("Set di semafori delle banchine creato con id = %d\n", sem_banchine_id);
	#endif

	/* definizione dell'argv dei porti */
	*argv_porti = (char *) malloc(MAX_STR_LEN*(QNT_PARAMETRI+1));
	for(i = 1; i < QNT_PARAMETRI+1; i++){
		argv_porti[i] = (char *) malloc(MAX_STR_LEN); 
		sprintf(argv_porti[i], "%d", PARAMETRO[i]);
	}
	argv_porti[QNT_PARAMETRI+1] = NULL;

	/*definizione dell'argv delle navi */
	*argv_navi = (char *) malloc(MAX_STR_LEN*(QNT_PARAMETRI));
	for(i = 0; i < QNT_PARAMETRI; i++){
		argv_navi[i] = (char *) malloc(MAX_STR_LEN); 
		sprintf(argv_navi[i], "%d", PARAMETRO[i]);
	}
	argv_navi[QNT_PARAMETRI] = NULL;

	/* creazione porti e navi... */
	for(i = 0; i < (PARAMETRO[SO_PORTI]+PARAMETRO[SO_NAVI]); i++){
		switch(childs[i] = fork()){
			case -1:
				TEST_ERROR
				break;
			case 0:	/* proc figlio */
				if(i < PARAMETRO[SO_PORTI]){
					sprintf(argv_porti[0], "%d", i);
					execl("porto", argv_porti);
				} else {
					sprintf(argv_navi[0], "%d", i);
					execl("porto", argv_navi);
				}
				break;
			default:
				break;
		}
	}
	/* avvio del timer della simulazione (+ sincronizzazione) */

	
	/* DUMP */

	/* WAIT di terminazione dei figli */

	/* de-allocazione risorse IPC */
	if(semctl(sem_banchine_id, 0, IPC_RMID) == 0)
		printf("Array banchine disallocato.\n");
	else{
		ERROR("nella deallocazione dell'array di banchine")
	}

	if(shmctl(dump_id, 0, IPC_RMID) == 0)
		printf("Memoria di dump disallocata.\n");
	else{
		ERROR("nella deallocazione della memoria di dump")
	}

	if(shmctl(porti_id, 0, IPC_RMID) == 0)
		printf("Memoria di stato dei porti disallocata.\n");
	else{
		ERROR("nella deallocazione della memoria di stato dei porti")
	}
	exit(EXIT_SUCCESS);
}