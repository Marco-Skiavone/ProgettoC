#include "definitions.h"
#include "my_lib.h"
#include "nave_lib.h"

int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){
	viaggio v;
	int id_dump; /*id memoria condivisa*/
	char *dump, *str;/*per fare shmat()*/
	int i,index;
	point position;
	if(argc != (3+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri alla nave")
		exit(EXIT_FAILURE);
	}
	index = atoi(argv[1]);
	TEST_ERROR
	for(i = 1; i < argc; i++){
		PARAMETRO[i] = atoi(argv[i]);
		TEST_ERROR
	}
	printf("NAVE %d: parametri letti.\n", getpid());
	STAMPA_PARAMETRI

	srand(SEED);
	

	/*agganciamento risorse
	- shmget()
	- shmat()
	nanosleep per il porto più vicino 
	*/
	id_dump = shmget(KEY_DUMP, SIZE_DUMP, 0666);
	dump = shmat(id_dump, NULL, 0);
	

	spostamento(v, &position);
	/*shmdt() a fine programma*/	
	exit(EXIT_SUCCESS);
}