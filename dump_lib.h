#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif


/* Calcolo dimensione dello spazio da allocare*/
int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI);

/* Alloca la shm necessaria al dump */
int alloca_shm_dump(int par_SO_MERCI, int par_SO_PORTI);

/* Ritorna l'indirizzo della memoria allocata */
void* indirizzoMemoriaDump();

/* Aggancia la memoria del dump */
void* aggancia_shm_dump();

/* Sgancia la memoria del dump */
int sgancia_shm_dump();

/* Cancella la shm del dump*/
int distruggi_shm_dump();