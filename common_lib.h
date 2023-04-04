#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#define _COMMON_LIB_H

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