#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* Crea la SHM e ne ritorna l'id. */
int alloca_shm(key_t key, size_t size);

/* Restituisce l'id della SHM con chiave 'key'. */
int find_shm(key_t key, size_t size);

/* Si aggancia alla SHM con id 'shmid'. */
void *aggancia_shm(int shmid);

/* Si sgancia dalla SHM. */
void sgancia_shm(void *shm_ptr);

/* Distrugge la SHM con id 'shmid'. */
void distruggi_shm(int shmid);