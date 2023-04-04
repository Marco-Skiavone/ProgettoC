#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "shm_lib.h"

int alloca_shm(key_t key, size_t size) {
    int shmid;
    if ((shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1) {
        perror("shmget alloca_shm");
        exit(255);
    }
    return shmid;
}

int find_shm(key_t key, size_t size) {
    int shmid;
    if ((shmid = shmget(key, size, S_IRUSR | S_IWUSR)) == -1) {
        perror("shmget find_shm");
        exit(255);
    }
    return shmid;
}

void *aggancia_shm(int shmid) {
    void *shm_ptr;
    if ((shm_ptr = shmat(shmid, NULL, 0)) == (void *)-1) {
        perror("shmat aggancia_shm");
        exit(255);
    }
    return shm_ptr;
}

void trova_tutti_ptr(void* p_mercato, void *p_lotti, void*p_posizioni, void *p_dump, int id_shms[], int PARAMETRO[]){
	if(id_shms == NULL){
		perror("trova_tutti_ptr: id_shms NULL!");
		exit(105);
	}
	p_mercato = aggancia_shm(id_shms[0]);
	TEST_ERROR
	p_lotti = aggancia_shm(id_shms[1]);
	TEST_ERROR
	p_posizioni = aggancia_shm(id_shms[2]);
	TEST_ERROR
	p_dump = aggancia_shm(id_shms[3]);
	TEST_ERROR
}

void sgancia_shm(void *shm_ptr) {
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt sgancia_shm");
        exit(255);
    }
}

void sgancia_risorse(void *ptr1, void *ptr2, void *ptr3, void *ptr4){
    sgancia_shm(ptr1);
    sgancia_shm(ptr2);
    sgancia_shm(ptr3);
    sgancia_shm(ptr4);
}

void distruggi_shm(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl distruggi_shm");
        exit(255);
    }
}