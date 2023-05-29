#include "sem_lib.h"

int sem_create(key_t key, int nsems) {
    int semid;
    if ((semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | S_IWUSR | S_IRUSR)) == -1) {
        fprintf(stderr, "semget creazione");
        exit(EXIT_FAILURE);
    }
    return semid;
}

void alloca_semafori(int *id_semaforo_banchine, int *id_semaforo_dump, int *id_semaforo_gestione, int *id_semaforo_mercato, int PARAMETRO[]){
    #ifdef DUMP_ME
    printf("SEM_CREATE_GESTIONE: %d\n", *id_semaforo_gestione = sem_create(CHIAVE_SEM_GESTIONE, 2));
    #else
    printf("SEM_CREATE_GESTIONE: %d\n", *id_semaforo_gestione = sem_create(CHIAVE_SEM_GESTIONE, 1));
    TEST_ERROR
    #endif
    printf("SEM_CREATE_BANCHINE: %d\n", *id_semaforo_banchine = sem_create(CHIAVE_SEM_BANCHINE, SO_PORTI));
    printf("SEM_CREATE_DUMP: %d\n", *id_semaforo_dump = sem_create(CHIAVE_SEM_DUMP, 2));
    printf("SEM_CREATE_MERCATO: %d\n", *id_semaforo_mercato = sem_create(CHIAVE_SEM_MERCATO, SO_PORTI));
    sem_set_all(*(id_semaforo_mercato), 1, SO_PORTI); /* bisogna farci il SET_ALL!!! (1 sola sys call)*/
    printf("__________________________ \n\n");
}

int sem_find(key_t key, int nsems) {
    int semid;
    if ((semid = semget(key, nsems, S_IWUSR | S_IRUSR)) == -1) {
        perror("semget find");
        exit(EXIT_FAILURE);
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
    }
    if(errno && errno != EINTR){
        perror("semop reserve");
        errno = 0;
    }
}

void sem_release(int semid, int sem_num) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    while (semop(semid, &sops, 1) == -1 && errno == EINTR) {
        printf("reitero su un nuovo sem_release\n");
    }
    if(errno && errno != EINTR){
        perror("semop release");
        errno = 0;
    }
}

void sem_wait_zero(int semid, int sem_num) {
    struct sembuf sops;
    sops.sem_num = sem_num;
    sops.sem_op = 0;
    sops.sem_flg = 0;
    errno = 0;
    while (semop(semid, &sops, 1) == -1 && errno == EINTR) {
    }
    if(errno && errno != EINTR){
        perror("semop wait for zero");
        errno = 0;
    }
}

void sem_set_val(int semid, int sem_num, int val) {
    if (semctl(semid, sem_num, SETVAL, val) == -1) {
        perror("semctl sem set val");
        exit(EXIT_FAILURE);
    }
}

void sem_set_all(int sem_id, int value, int arr_size){
    int semval, i;
    union semun arg;
    arg.array = malloc(arr_size * sizeof(int));
    for(i = 0; i < arr_size; i++){
        arg.array[i] = value; 
    }
    semval = semctl(sem_id, 0, SETALL, arg);
    if (semval == -1) {
        perror("semctl sem set all");
        exit(EXIT_FAILURE);
    }
    free(arg.array);
}

int sem_get_val(int sem_id, int sem_num) {
    int semval;
    semval = semctl(sem_id, sem_num, GETVAL);
    if (semval == -1) {
        perror("semctl sem get val");
        exit(EXIT_FAILURE);
    }
    return semval;
}

void sem_destroy(int semid) {
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl sem destroy");
        exit(EXIT_FAILURE);
    }
}

void inizializza_semafori(int *id_mercato, int *id_gestione, int *id_banchine, int *id_dump,int PORTI){
    *id_mercato = sem_find(CHIAVE_SEM_MERCATO, PORTI);
    #ifdef DUMP_ME
    *id_gestione = sem_find(CHIAVE_SEM_GESTIONE, 2);
    #else
    *id_gestione = sem_find(CHIAVE_SEM_GESTIONE, 1);
    #endif
    *id_banchine = sem_find(CHIAVE_SEM_BANCHINE, PORTI);
    *id_dump = sem_find(CHIAVE_SEM_DUMP,2);
}