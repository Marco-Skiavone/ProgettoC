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

int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){

}