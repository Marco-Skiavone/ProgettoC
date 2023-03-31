#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "nave_lib.h"

void richiedi_banchina(int id_semaforo_banchine, int indice_porto){
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    sem_reserve(id_semaforo_banchine, indice_porto);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void stato_nave(int stato, int id_semaforo_dump, void *vptr_shm_dump, int indice){
	switch(stato){
        case DN_MV_PORTO:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviscariche--;
            ((dump*)vptr_shm_dump)->nd.naviporto++;
            sem_release(id_semaforo_dump,1);
            printf("0. stato nave %d aggiornato\n", indice);
            break;
        case DN_MC_PORTO:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.navicariche--;
            ((dump*)vptr_shm_dump)->nd.naviporto++;
            sem_release(id_semaforo_dump,1);
            printf("1. stato nave %d aggiornato\n", indice);
            break;
        case DN_PORTO_MV:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviporto--;
            ((dump*)vptr_shm_dump)->nd.naviscariche++;
            sem_release(id_semaforo_dump,1);
            printf("2. stato nave %d aggiornato\n", indice);
            break;
        case DN_PORTO_MC: 
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviporto--;
            ((dump*)vptr_shm_dump)->nd.navicariche++;
            sem_release(id_semaforo_dump,1);
            printf("3. stato nave %d aggiornato\n", indice);
            break;
        default:
            perror("**** ERRORE! Caso default di stato_nave()");
            break;
    }
}


