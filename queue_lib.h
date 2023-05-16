#ifndef _QUEUE_LIB_H
#define _QUEUE_LIB_H
#include "definitions.h"

/* Crea la coda di msg data una chiave e ne restituisce l'id. */
int set_coda(key_t key);

/* Restituisce l'id della coda richieste già creata. */
int get_coda_id(key_t key);

/* Distrugge la coda... (commento inutile) */
void distruggi_coda(int coda_id);

/** Invia una richiesta 'r' alla coda. 
 * @param fd_fifo è il file descriptor della FIFO per passare le richieste in eccesso.
 * @note i metodi invia_richiesta() e accetta_richiesta() riescono a gestire il mtype.
 * In pratica aggiungono aggiungono 1 prima di eseguire effettivamente la system call, 
 * in questa maniera è possibile usare l'indice dei porti (che parte da 0) come tipo dei messaggi. */
void invia_richiesta(richiesta r, int coda_id, int fd_fifo);

/* Riceve una richiesta 'r' dalla coda. 
* 
* NB: i metodi invia_richiesta() e accetta_richiesta() riescono a gestire il mtype.
* In pratica aggiungono aggiungono 1 prima di eseguire effettivamente la system call,
* in questa maniera è possibile usare l'indice dei porti (che parte da 0) come tipo dei messaggi. */
richiesta accetta_richiesta(int msgtype, int coda_id);
#endif