#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
/* Quanti byte può occupare una riga di parametri del file di configurazione. */
#define MAX_FILE_STR_LEN 60
/* Indica quanti parametri vanno inseriti a tempo di esecuzione. */
#define QNT_PARAMETRI 13

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

/* parametri */
#define SO_NAVI 0
#define SO_PORTI 1
#define SO_MERCI 2
#define SO_SIZE 3
#define SO_MIN_VITA 4
#define SO_MAX_VITA 5
#define SO_LATO 6
#define SO_SPEED 7
#define SO_CAPACITY 8
#define SO_BANCHINE 9
#define SO_FILL 10
#define SO_LOADSPEED 11
#define SO_DAYS 12

int main(int argc, char *argv[]){
	int NUM_RIGA_FILE, i, file_config_char;
	FILE *file_config;
	char *str = (char *) malloc(MAX_FILE_STR_LEN);
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
	
	#ifdef DEBUG
		printf("\nInizializzato progetto con flag di debug...\n");
	#endif
		
}
