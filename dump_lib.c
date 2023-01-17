#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "dump_lib.h"

/* Puntatore del DUMP*/
static struct dump *dump_ptr;
/* id del DUMP*/
static int shm_dump;

/* funzione interna per calcolare lo spazio necessario per il dump */
int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI){
    int spazio;
    spazio = sizeof( porto_dump) * par_SO_PORTI;
    spazio += sizeof( merce_dump) * par_SO_MERCI;
    spazio += sizeof( dump);
    return spazio;
}

/* Crea il dump e ne ritorna il puntatore */
int allocaMemoriaDump(int par_SO_MERCI, int par_SO_PORTI){
    int spazio = calcola_spazio_necessario(par_SO_MERCI, par_SO_PORTI);
    shm_dump = semget(KEY_DUMP, spazio, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    dump_ptr = shmat(shm_dump, NULL, 0);
    TEST_ERROR
    return shm_dump;
}

/* Ritorna il puntatore del dump */
void* indirizzoMemoriaDump(){
    return dump_ptr;
}

void* agganciaMemoriaDump(){
    void* ret_val = shmat(shm_dump, NULL, 0);
    TEST_ERROR
    return ret_val;
}

int sganciaMemoriaDump(){
    int ret_val = shmdt(dump_ptr);
    TEST_ERROR
    return ret_val;
}

int distruggiMemoriaDump(){
    int return_val = shmctl(shm_dump, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}