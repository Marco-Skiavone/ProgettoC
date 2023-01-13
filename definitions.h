#define _GNU_SOURCE
#define _DEFINITIONS_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>


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

/* MACRO per riferirsi ai parametri */
#define SO_NAVI PARAMETRO[I_NAVI]
#define SO_PORTI PARAMETRO[I_PORTI]
#define SO_MERCI PARAMETRO[I_MERCI]
#define SO_SIZE PARAMETRO[I_SIZE]
#define SO_MIN_VITA PARAMETRO[I_MIN_VITA]
#define SO_MAX_VITA PARAMETRO[I_MAX_VITA]
#define SO_LATO PARAMETRO[I_LATO]
#define SO_SPEED PARAMETRO[I_SPEED]
#define SO_CAPACITY PARAMETRO[I_CAPACITY]
#define SO_BANCHINE PARAMETRO[I_BANCHINE]
#define SO_FILL PARAMETRO[I_FILL]
#define SO_LOADSPEED PARAMETRO[I_LOADSPEED]
#define SO_DAYS PARAMETRO[I_DAYS]

#define PERMESSI 		\
S_IWUSR | S_IRUSR /* | S_IRGRP | S_IWGRP| S_IROTH | S_IWOTH */

/* Indica quanti parametri vanno inseriti a tempo di esecuzione. */
#define QNT_PARAMETRI 13

/* Quanti byte può occupare una riga di parametri del file di configurazione. */
#define MAX_FILE_STR_LEN 60

/* Indica il massimo di byte (cifre) per rappresentare
 *  un parametro in input ai processi figlio */
#define MAX_STR_LEN 15

/* key della memoria contenente i parametri */
#define KEY_PARAMETRI 01

/* i byte necessari per la memoria parametri */
#define SIZE_PARAMETRI (QNT_PARAMETRI*sizeof(int))

/* key della memoria di dump (shm) */
#define KEY_DUMP 11

/* i byte necessari per il dump (shm) */
#define SIZE_DUMP 100

/* n. di byte usati da ogni porto nella shm "registro porti" */
#define SIZE_PORTO_IN_MERCATO 100

/* key della memoria del mercato (shm) */
#define KEY_MERCATO 12

/* i byte necessari per la memoria mercato (shm) */
#define SIZE_MERCATO (SO_PORTI*SIZE_PORTO_IN_MERCATO)

/* key della memoria posizioni */
#define KEY_POSIZIONI 13

/* i byte necessari per la memoria posizioni (shm) */
#define SIZE_POSIZIONI sizeof(point)*SO_PORTI

/* key della coda di richieste */
#define KEY_CODA_RICHIESTE 21

/* chiave per definire la shm dei lotti */
#define KEY_LOTTI 66

#define SIZE_LOTTI 

/* size di ogni messaggio in coda richieste */
#define SIZE_MSG sizeof(int)*2

/* key del set di semafori per gestire le banchine */
#define KEY_BANCHINE_SEM 31

/* tolleranza per vedere se due porti sono accettabilmente distanti fra loro */
#define TOLLERANZA 0.05

typedef struct _position {
	double x;
	double y;
} point;

/* struct di composizione di ogni tipo di merce, porto e info delle navi
typedef struct _info_merce {
	int in_nave;
	int in_porto;
	int m_consegnata;
	int m_scaduta_porto;
	int m_scaduta_nave;
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
} info_nave;*/

typedef struct { /*struct ritornata da porto_piu_vicino*/
	int indice_porto; /*i del ciclo*/
	int nanosec_nano;
	/*restituisco anche le coordinate del porto dove si troverà la nave dopo*/
	double x;
	double y;
} viaggio;

/* Rappresenta un lotto di merce */
typedef struct {
    int val;
    int exp;
} merce;

/* richiesta da inserire in CODA MSG*/
typedef struct {
    long mtype;
    int indicemerce;
    int nlotti;
} richiesta;



/* abbiamo definito qui il seed delle generazioni
 randomiche che possiamo usare nelle varie simulazioni */
#define SEED getpid()