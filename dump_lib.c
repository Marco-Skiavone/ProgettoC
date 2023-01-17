#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

#include "dump_lib.h"
#define chiavedump 99

static struct dump *dump_ptr;
static int shm_dump;

int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI){
    int spazio;
    spazio = sizeof(struct porto_dump) * par_SO_PORTI;
    spazio += sizeof(struct merce_dump) * par_SO_MERCI;
    spazio += sizeof(struct dump);
    return spazio;
}

int allocaMemoriaDump(int par_SO_MERCI, int par_SO_PORTI){
    int spazio = calcola_spazio_necessario(par_SO_MERCI, par_SO_PORTI);
    shm_dump = semget(chiavedump, spazio, IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR
    dump_ptr = shmat(shm_dump, NULL, 0);
    TEST_ERROR
    return shm_dump;
}

void* indirizzoMemoriaDum(){
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