#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "common_lib.h"

/** Questo modulo C serve a compilare i metodi comuni a più processi. */

void trova_tutti_id(int *id_mercato, int *id_lotti, int *id_posizioni, int *id_dump, int *id_coda, int PARAMETRO[]){
    *(id_mercato) = find_shm(CHIAVE_SHAREDM_MERCATO, SIZE_SHAREDM_MERCATO);
    TEST_ERROR
    *(id_lotti) = find_shm(CHIAVE_SHAREDM_DETTAGLI_LOTTI, SIZE_SHAREDM_DETTAGLI_LOTTI);
    TEST_ERROR
    *(id_posizioni) = find_shm(CHIAVE_SHAREDM_POSIZIONI_PORTI, SIZE_SHAREDM_POSIZIONI_PORTI);
    TEST_ERROR
    *(id_dump) = find_shm(CHIAVE_SHAREDM_DUMP, SIZE_SHAREDM_DUMP);
    TEST_ERROR
	*(id_coda) = get_coda_id(CHIAVE_CODA);
}