#include "definitions.h"
#include "my_lib.h"
#include "merci_lib.h"

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){
	int i, index;
	/*1 = # porto (per array banchine), #[QNT_PARAMETRI] parametri */
	if(argc != (3+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri al porto")
	}
	index = atoi(argv[0]);
	TEST_ERROR
	for(i = 1; i < argc; i++){
		PARAMETRO[i] = atoi(argv[i]);
		TEST_ERROR
	}
	printf("PORTO %d: parametri letti.\n", getpid());
	STAMPA_PARAMETRI

	srand(time(NULL));

	/* merce (*deposito)[SO_MERCI] = indirizzoMercato();*/
	
	/* spawnMerciPorti((SO_FILL/SO_PORTI), SO_MERCI, deposito, index, ); */
	exit(EXIT_SUCCESS);
}