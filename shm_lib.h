#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

int alloca_shm(key_t key, size_t size);

int find_shm(key_t key, size_t size);

void *aggancia_shm(int shmid);

void sgancia_shm(void *shm_ptr);

void distruggi_shm(int shmid);