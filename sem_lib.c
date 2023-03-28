#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "sem_lib.h"

int sem_create(key_t key, int nsems) {
    int semid;
    if ((semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1) {
        perror("semget creazione");
        exit(255);
    }
    return semid;
}

int sem_find(key_t key, int nsems) {
    int semid;
    if ((semid = semget(key, nsems, S_IWUSR | S_IRUSR)) == -1) {
        perror("semget find");
        exit(255);
    }
    return semid;
}

void sem_reserve(int semid, int sem_num) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    while (semop(semid, &sops, 1) == -1 && errno == EINTR) {
        printf("reitero su un nuovo sem_reserve\n");
        perror("semop reserve");
    }/*
    if(semop(semid, &sops, 1) == -1 && errno == EINTR){
        printf("apro un nuovo sem_reserve\n");
        sem_reserve(semid, sem_num);
    }*/
}

void sem_release(int semid, int sem_num) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    while (semop(semid, &sops, 1) == -1 && errno == EINTR) {
        printf("reitero su un nuovo sem_release\n");
        perror("semop release");
    }/*
    if(semop(semid, &sops, 1) == -1 && errno == EINTR){
        printf("apro un nuovo sem_release\n");
        sem_release(semid, sem_num);
    }*/
}

void sem_wait_zero(int semid, int sem_num) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = 0;
    sops.sem_flg = 0;
    errno = 0;/*
    while (semop(semid, &sops, 1) == -1 && errno == EINTR) {
        perror("semop wait for zero");
    }
    */
    if(semop(semid, &sops, 1) == -1 && errno == EINTR){
        printf("apro un nuovo sem_wait_zero\n");
        sem_wait_zero(semid, sem_num);
    }
}

void sem_set_val(int semid, int sem_num, int val) {
    if (semctl(semid, sem_num, SETVAL, val) == -1) {
        perror("semctl sem set val");
        exit(255);
    }
}

int sem_get_val(int sem_id, int sem_num) {
    int semval;
    semval = semctl(sem_id, sem_num, GETVAL);
    if (semval == -1) {
        perror("semctl sem get val");
        exit(255);
    }
    return semval;
}

void sem_destroy(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl sem destroy");
        exit(255);
    }
}

void inizializza_semafori(int *id_mercato, int *id_gestione, int *id_banchine, int *id_dump,int PORTI){
    *id_mercato = sem_find(CHIAVE_SEM_MERCATO, PORTI);
    *id_gestione = sem_find(CHIAVE_SEM_GESTIONE, 2);
    *id_banchine = sem_find(CHIAVE_SEM_BANCHINE, PORTI);
    *id_dump = sem_find(CHIAVE_SEM_DUMP,2);
}