#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

#ifndef _SHM_LIB_H
#define _SHM_LIB_H
#endif
/* Crea la SHM e ne ritorna l'id. */
int alloca_shm(key_t key, size_t size);

/* Restituisce l'id della SHM con chiave 'key'. */
int find_shm(key_t key, size_t size);

/* Si aggancia alla SHM con id 'shmid'. */
void *aggancia_shm(int shmid);

/* Si sgancia dalla SHM. */
void sgancia_shm(void *shm_ptr);

/* Sgancia i 4 puntatori alle shm della simulazione. Richiama al suo interno sgancia_shm()*/
void sgancia_risorse(void *ptr1, void *ptr2, void *ptr3, void *ptr4);

/* Distrugge la SHM con id 'shmid'. */
void distruggi_shm(int shmid);