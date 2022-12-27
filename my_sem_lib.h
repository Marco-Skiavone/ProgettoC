#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#ifndef _STDIO_H
	#include <stdio.h>
#endif
#ifndef _STDLIB_H
	#include <stdlib.h>
#endif
#ifndef _SYS_TYPES_H
	#include <sys/types.h>
#endif
#ifndef _SYS_IPC_H
	#include <sys/ipc.h>
#endif
#ifndef _SYS_SEM_H
	#include <sys/sem.h>
#endif
#ifndef _STRING_H
	#include <string.h>
#endif

/*
 * The following union must be defined as required by the semctl man
 * page
 */
union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};


/*
 * Set a semaphore to a user defined value
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * - sem_val: the initialization value of the semaphore
 * RESULT:
 * - the selected semaphore is initialized to the given value
 * - the returned value is the same as the invoked semctl
 */
int sem_set_val(int sem_id, int sem_num, int sem_val);

/*
 * Try to access the resource
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * RESULT
 * - if the resource is available (semaphore value > 0), the semaphore
 *   is decremented by one
 * - if the resource is not available (semaphore value == 0), the
 *   process is blocked until the resource becomes available again
 * - the returned value is the same as the invoked semop
 */
int sem_reserve(int sem_id, int sem_num);

/*
 * Release the resource
 * INPUT:
 * - sem_id: the ID of the semaphore IPC object
 * - sem_num: the position of the semaphore in the array
 * RESULT:
 * - the semaphore value is incremented by one. This may unblock some
 *   process
 * - the returned value is the same as the invoked semop
 */
int sem_release(int sem_id, int sem_num);

/* Print all semaphore values to a string */
int sem_getall(char * my_string, int sem_id);

/* Set all n_sems semaphore values to value */
int sem_setall(int n_sems, int value, int sem_id);