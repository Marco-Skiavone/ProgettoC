#ifndef _COMMON_LIB_H
#define _COMMON_LIB_H
#include "definitions.h"
#include "shm_lib.h"
#include "queue_lib.h"

/** Questo modulo serve a compilare i metodi comuni a più processi.
 * ---------------------------------------------------------------- */

/* Esegue le find_shm() sugli interi passati per referenza */
void trova_tutti_id(int *id_mercato, int *id_lotti, int *id_posizioni, int *id_dump, int *id_coda, int PARAMETRO[]);

/* Esegue le aggancia_shm() sui puntatori passati come argomenti della funzione */
void aggancia_tutte_shm(void **p_mercato, void **p_lotti, void **p_posizioni, void **p_dump, int IDS[], int PARAMETRO[]);


#endif