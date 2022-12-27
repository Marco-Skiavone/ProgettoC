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
	FILE *file_config;
	char *str = (char *) malloc(MAX_FILE_STR_LEN);
	char *dump_p = (char *) malloc(SIZE_DUMP);
	char *porti_p = (char *) malloc(SIZE_MEM_PORTI);
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
	}
	
	/* creazione della shm di dump */
	dump_id = shmget(DUMP_KEY, SIZE_DUMP, IPC_CREAT | IPC_EXCL);
	TEST_ERROR
	dump_p = shmat(dump_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Dump creato con id = %d\n", dump_id);
	#endif

	/* creazione della shm "registro dei porti" */
	porti_id = shmget(PORTI_MEM_KEY, SIZE_MEM_PORTI, IPC_CREAT | IPC_EXCL);
	TEST_ERROR
	porti_p = shmat(dump_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Registro porti creato con id = %d\n", porti_id);
	#endif

	/* creazione del set di semafori "banchine dei porti" */
	sem_banchine_id = semget(BANCHINE_SEM_KEY, PARAMETRO[SO_PORTI], IPC_CREAT | IPC_EXCL);
	TEST_ERROR
	sem_setall(PARAMETRO[SO_PORTI], PARAMETRO[SO_BANCHINE], sem_banchine_id);
	#ifdef DEBUG
		printf("Set di semafori delle banchine creato con id = %d\n", sem_banchine_id);
	#endif


}