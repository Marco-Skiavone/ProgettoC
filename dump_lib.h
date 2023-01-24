#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif


/* Calcolo dimensione dello spazio da allocare*/
int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI);

/* Alloca la shm necessaria al dump */
int alloca_shm_dump(int par_SO_MERCI, int par_SO_PORTI);

/* fa la get del dump senza crearlo */
int set_shm_dump(int MERCI, int PORTI);