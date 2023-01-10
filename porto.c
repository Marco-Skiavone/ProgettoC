#include "definitions.h"
#include "my_lib.h"

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){
	int i, index;

	/*1 = # porto (per array banchine), #[QNT_PARAMETRI] parametri */
	if(argc != (1+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri al porto")
	}
	index = atoi(argv[0]);
	TEST_ERROR
	for(i = 1; i < argc; i++){
		PARAMETRO[i] = atoi(argv[i]);
		TEST_ERROR
	}
	printf("PORTO %d: parametri letti.\n", getpid());
	srand(SEED);
	

	exit(EXIT_SUCCESS);
}