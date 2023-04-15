#ifndef _COMMON_LIB_H
#define _COMMON_LIB_H
#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#ifndef _SHM_LIB_H
#include "shm_lib.h"
#endif
#ifndef _QUEUE_LIB_H
#include "queue_lib.h"
#endif

/** Questo modulo serve a compilare i metodi comuni a più processi.
 * ---------------------------------------------------------------- */

/* Esegue le find_shm(), ritornando i valori negli indirizzi passati come argomenti. */
void trova_tutti_id(int *id_mercato, int *id_lotti, int *id_posizioni, int *id_dump, int *id_coda, int PARAMETRO[]);

/* Esegue le aggancia_shm(), ritornando i valori negli indirizzi passati come argomenti. */
void aggancia_tutte_shm(void **p_mercato, void **p_lotti, void **p_posizioni, void **p_dump, int IDS[], int PARAMETRO[]);
#endif