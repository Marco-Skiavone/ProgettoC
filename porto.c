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
	int i, i_banchine_array;

	/*1 = # porto (per array banchine), #[QNT_PARAMETRI] parametri */
	if(argc != (1+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri al porto")
	}
	i_banchine_array = atoi(argv[0]);
	TEST_ERROR
	for(i = 1; i < argc; i++){
		PARAMETRO[i] = atoi(argv[i]);
		TEST_ERROR
	}
	printf("PORTO %d: parametri letti.\n", getpid());
	srand(getpid());
	



	exit(EXIT_SUCCESS);
}