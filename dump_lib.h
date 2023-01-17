#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif


/* Calcolo dimensione dello spazio da allocare*/
int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI);

/* Alloca la shm necessaria al dump */
int allocaMemoriaDump(int par_SO_MERCI, int par_SO_PORTI);

/* Ritorna l'indirizzo della memoria allocata */
void* indirizzoMemoriaDum();

/* Aggancia la memoria del dump */
void* agganciaMemoriaDump();

/* Sgancia la memoria del dump */
int sganciaMemoriaDump();

/* Cancella la shm del dump*/
int distruggiMemoriaDump();