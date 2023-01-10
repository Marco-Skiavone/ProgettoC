#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>

/* stampa un messaggio di errore dove str è una stringa personalizzabile */
#define ERROR(str)											\
	fprintf(stderr, "\nErrore %s a linea %d!\n", str, __LINE__);

/* controlla il valore di errno e nel caso lo segnala */
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

/* indice dei parametri */
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

/* Quanti byte può occupare una riga di parametri del file di configurazione. */
#define MAX_FILE_STR_LEN 60

/* Indica quanti parametri vanno inseriti a tempo di esecuzione. */
#define QNT_PARAMETRI 13

/* Indica il massimo di byte (cifre) per rappresentare
 *  un parametro in input ai processi figlio */
#define MAX_STR_LEN 15

/* i byte necessari per il dump (shm) */
#define SIZE_DUMP 100

/* key della memoria di dump (shm) */
#define KEY_DUMP 11

/* n. di byte usati da ogni porto nella shm "registro porti" */
#define SIZE_PORTO_IN_MERCATO 100

/* i byte necessari per la memoria mercato (shm) */
#define SIZE_MERCATO (PARAMETRO[SO_PORTI]*SIZE_PORTO_IN_MERCATO)

/* key della memoria del mercato (shm) */
#define KEY_MERCATO 12

/* key della memoria posizioni */
#define KEY_POSIZIONI 13

/* i byte necessari per la memoria posizioni (shm) */
#define SIZE_POSIZIONI sizeof(point)*PARAMETRO[SO_PORTI]

/* key della coda di richieste */
#define KEY_CODA_RICHIESTE 21

/* key del set di semafori per gestire le banchine */
#define KEY_BANCHINE_SEM 31

typedef struct _position{
	double x;
	double y;
} point;

/* abbiamo definito qui il seed delle generazioni
 randomiche che possiamo usare nelle varie simulazioni */
#define SEED getpid()