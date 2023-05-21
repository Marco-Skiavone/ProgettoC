#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#include "demone.h"

/* define utilizzata per effettuare test di debug*/
#define STAMPA_DEBUG printf("%s: data = %d, linea %d\n", __FILE__, CAST_DUMP(VPTR_SHM_DUMP)->data, __LINE__);

/* stampa un messaggio di errore dove str è una stringa personalizzabile */
#define ERROR(str)											\
	fprintf(stderr, "\nErrore %s a linea %d! PID:%d\n", str, __LINE__, getpid());

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
		errno=0;							\
	}

/* Stampa i parametri */
#define STAMPA_PARAMETRI 				\
	printf("Parametri:\nSO_NAVI | SO_PORTI | SO_MERCI | SO_SIZE | SO_MIN_VITA | SO_MAX_VITA | "); 	\
	printf("SO_LATO | SO_SPEED | SO_CAPACITY | SO_BANCHINE | SO_FILL | SO_LOADSPEED | SO_DAYS\n"); 	\
	for (i = 0; i < QNT_PARAMETRI; i++) {			\
		printf(" %d ", PARAMETRO[i]);		\
		if (i != QNT_PARAMETRI - 1)			\
			printf("|");					\
		else								\
			printf("\n");					\
	}										
	

/* stati nave */
#define DN_MV_PORTO 0	/* da "in mare vuota" a "in porto" */
#define DN_MC_PORTO 1	/* da "in mare carica" a "in porto" */
#define DN_PORTO_MV 2	/* da "in porto" a "in mare vuota" */
#define DN_PORTO_MC 3	/* da "in porto" a "in mare carica" */

#define NAVE_IN_PORTO 0
#define NAVE_IN_MARE 1

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
#define I_STORM_DURATION 13
#define I_SWELL_DURATION 14
#define I_MAELSTORM 15

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
#define SO_STORM_DURATION PARAMETRO[I_STORM_DURATION]
#define SO_SWELL_DURATION PARAMETRO[I_SWELL_DURATION]
#define SO_MAELSTORM PARAMETRO[I_MAELSTORM]

/* MACRO per riferimenti agli ID dei semafori*/
#define ID_SEMAFORO_BANCHINE SEM_ID[0]
#define ID_SEMAFORO_DUMP SEM_ID[1]
#define ID_SEMAFORO_GESTIONE SEM_ID[2]
#define ID_SEMAFORO_MERCATO SEM_ID[3]

/* MACRO per riferimenti ai puntatori delle shared memory*/
#define VPTR_SHM_DETTAGLI_LOTTI VPTR_ARR[0]
#define VPTR_SHM_DUMP VPTR_ARR[1]
#define VPTR_SHM_MERCATO VPTR_ARR[2]
#define VPTR_SHM_POSIZIONI_PORTI VPTR_ARR[3]


/* Numero di parametri: 
 * - 13 versione da 24
 * - 16 versione da 30 */
#define QNT_PARAMETRI 16

/* MACRO utilizzate per indicare le chiavi delle shared memory, la loro dimensione,
	la dimensione dei messaggi e le chiavi dei semafori */
#define CHIAVE_SHAREDM_MERCATO 10
#define SIZE_SHAREDM_MERCATO ((sizeof(merce) * SO_MERCI ) * SO_PORTI)

#define CHIAVE_SHAREDM_POSIZIONI_PORTI 20
#define SIZE_SHAREDM_POSIZIONI_PORTI ((sizeof(point) * SO_PORTI))

#define CHIAVE_SHAREDM_DETTAGLI_LOTTI 30
#define SIZE_SHAREDM_DETTAGLI_LOTTI (sizeof(merce) * SO_MERCI)

#define CHIAVE_SHAREDM_DUMP 40
#define SIZE_SHAREDM_DUMP ((sizeof(porto_dump) * SO_PORTI) + (sizeof(merce_dump) * SO_MERCI) + sizeof(dump))

#define CHIAVE_SEM_MERCATO 11
#define CHIAVE_SEM_DUMP 41
#define CHIAVE_SEM_BANCHINE 51
#define CHIAVE_SEM_GESTIONE 61

/* --- casting delle shm --- */
/* usate per fare il casting dei void pointer alla shared memory*/

#define CAST_MERCATO(ptr) \
	((merce(*)[SO_MERCI])ptr)

#define CAST_POSIZIONI_PORTI(ptr) \
	((point *)ptr)

#define CAST_DETTAGLI_LOTTI(ptr) \
	((merce *)ptr)

#define CAST_DUMP(ptr) \
	((dump *) ptr)

#define CAST_MERCE_DUMP(ptr) \
	((merce_dump *) ptr+sizeof(int))

#define CAST_PORTO_DUMP(ptr) \
	((porto_dump*)(((merce_dump*) ptr+sizeof(int))+SO_MERCI))

#define CAST_TERM_DUMP(ptr)	\
	(((dump*)ptr)->term_dump)

/* ------------------------- */

/* Lunghezza massima di ogni stringa di argv_figli[x].
 * Usata per passare i parametri ai processi figli. */
#define MAX_STR_LEN 15

/* Tolleranza della uguaglianza tra punti nella mappa */
#define TOLLERANZA 0.05

/* Massimo di richieste da poter leggere. (arbitrario: 20 ?) */
#define MAX_REQ_LETTE ((SO_PORTI/SO_NAVI) >= 10 ? SO_PORTI/SO_NAVI : 10)

/* Massima lunghezza dell'array di carico merci delle navi. */
#define MAX_CARICO 10

/* Punto generico di posizione con coordinate x e y. */
typedef struct {
	double x;
	double y;
} point;

/* Rappresenta un lotto di merce. */
typedef struct {
    int val;
    int exp;
} merce;

/* Tipo specifico per il carico in nave, possiede un indice che è l'indice della merce. */
typedef struct {
	/* indice della merce */
    int indice; 
    merce mer;
} merce_nave;

/* SERIE DI STRUCT NECESSARIE PER I DATI RIGUARDANTI I DUMP*/

/* Tipo usato per contare lo stato delle navi nel dump. */
typedef struct {
    int naviporto;
    int naviscariche;
    int navicariche;
} nave_dump;

/* Tipo usato per la struttura del vettore di porti del dump. */
typedef struct {
    int mercespedita;
    int mercericevuta;
    int mercepresente;
    int banchineoccupate;
    int banchinetotali;
} porto_dump ;

/* Tipo usato per la struttura del vettore di merci del dump. */
typedef struct {
    int presente_in_porto;
    int presente_in_nave;
    int consegnata;
    int scaduta_in_porto;
    int scaduta_in_nave;
} merce_dump;

/* Tipo usato per la struttura del vettore nel dump, usato in fase di terminazione della simulazione. */
typedef struct {
	int porto_spedite;	/* Indica il porto che a fine simulazione ha spedito più merci. */
	int porto_ricevute;	/* Indica il porto che a fine simulazione ha ricevuto più merci. */
} term_dump;

/* Tipo usato per la struttura del dump. */
typedef struct {
	int data;
    merce_dump *merce_dump_ptr;
    porto_dump *porto_dump_ptr;
	term_dump term_dump;
    nave_dump nd;
	int rand_porti;
} dump;

#endif