#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "dump_lib.h"

/* funzione interna per calcolare lo spazio necessario per il dump */
int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI){
    int spazio;
    spazio = sizeof( porto_dump) * par_SO_PORTI;
    spazio += sizeof( merce_dump) * par_SO_MERCI;
    spazio += sizeof( dump);
    return spazio;
}

/* Crea il dump e ne ritorna il puntatore */
int alloca_shm_dump(int par_SO_MERCI, int par_SO_PORTI){
    int spazio = calcola_spazio_necessario(par_SO_MERCI, par_SO_PORTI);
    int id_dump = shmget(KEY_DUMP, spazio, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    return id_dump;
}


/* fa la get del dump senza crearlo */
int set_shm_dump(int MERCI, int PORTI){
    int id_dump = shmget(KEY_DUMP, calcola_spazio_necessario(MERCI, PORTI), PERMESSI);
    TEST_ERROR
    return id_dump;
}