#include "definitions.h"
#include "my_lib.h"

int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){
	viaggio v;
	point *p;/*puntatore alla posizione della nave*/
	int id; /*id memoria condivisa*/
	char * dump;/*per fare shmat()*/
	int i, index;
	point position;
	if(argc != (1+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri alla nave")
		exit(EXIT_FAILURE);
	}
	index = atoi(argv[0]);
	TEST_ERROR
	for(i = 1; i < argc; i++){
		PARAMETRO[i] = atoi(argv[i]);
		TEST_ERROR
	}

	printf("NAVE %d: parametri letti.\n", getpid());
	/*
		debug
	*/
	exit(0);
	srand(SEED);
	/*generazione punto casuale (random lib.c)*/
	/*agganciamento risorse
	- shmget()
	- shmat()
	nanosleep per il porto più vicino 
	*/
	id = shmget(KEY_DUMP, SIZE_DUMP, 0666);
	dump = shmat(id, NULL, 0);
	/*...*/
	/*nanosleep();*/

	spostamento(v, p);
	/*shmdt() a fine programma*/	
	exit(EXIT_SUCCESS);


}


