#include "definitions.h"
#include "my_lib.h"

int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){
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
	srand(SEED);


	exit(EXIT_SUCCESS);
}