#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

int sem_create(key_t key, int nsems);

int sem_find(key_t key, int nsems);

void sem_reserve(int semid, int sem_num);

void sem_release(int semid, int sem_num);

void sem_wait_zero(int semid, int sem_num);

void sem_set_val(int semid, int sem_num, int val);

void sem_destroy(int semid);

int sem_get_val(int sem_id, int sem_num);