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
#define I_NAVI 0
#define I_PORTI 1
#define I_MERCI 2
#define I_SIZE 3
#define I_MIN_VITA 4
#define I_MAX_VITA 5
#define I_LATO 6
#define I_SPEED 7
#define I_CAPACITY 8
#define I_BANCHINE 9
#define I_FILL 10
#define I_LOADSPEED 11
#define I_DAYS 12

#define SO_NAVI(PAR) PAR->S_NAVI
#define SO_PORTI parametro->S_PORTI
#define SO_MERCI parametro->S_MERCI
#define SO_SIZE parametro->S_SIZE
#define SO_MIN_VITA parametro->S_MIN_VITA
#define SO_MAX_VITA parametro->S_MAX_VITA
#define SO_LATO parametro->S_LATO
#define SO_SPEED parametro->S_SPEED
#define SO_CAPACITY parametro->S_CAPACITY
#define SO_BANCHINE parametro->S_BANCHINE
#define SO_FILL parametro->S_FILL
#define SO_LOADSPEED parametro->S_LOADSPEED
#define SO_DAYS parametro->S_DAYS

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

/* indica lastructusata per la gestione dei parametri (shm) */
typedef struct _par {
	int S_NAVI;
	int S_PORTI;
	int S_MERCI;
	int S_SIZE;
	int S_MIN_VITA;
	int S_MAX_VITA;
	int S_LATO;
	int S_SPEED;
	int S_CAPACITY;
	int S_BANCHINE;
	int S_FILL;
	int S_LOADSPEED;
	int S_DAYS;
} par;

typedef struct _position{
	double x;
	double y;
} point;

/* struct di composizione del dump */
typedef struct _shm_dump {
	info_merce merce[SO_MERCI];
	info_porto porti[SO_PORTI];
	info_nave nave;
} shm_dump;


/* struct di composizione di ogni tipo di merce */
typedef struct _info_merce {
	int in_nave;
	int in_porto;
	int m_consegnata;
	int m_scaduta_porto;
	int m_scaduta_nave
} info_merce;

typedef struct _info_porto {
	int m_spedita;
	int m_ricevuta;
	int m_scaduta;
} info_porto;

typedef struct _info_nave {
	int in_mare_con_carico;
	int in_mare_vuota;
	int in_porto;
} info_nave;


/* abbiamo definito qui il seed delle generazioni
 randomiche che possiamo usare nelle varie simulazioni */
#define SEED getpid()

