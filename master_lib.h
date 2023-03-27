#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* Funzione d'ausilio per controllare che sia corretto il dump:
 * - Controlla che mercato_shm sia uguale a dump_shm
 * RITORNA: 1 se è tutto ok, 0 altrimenti. (viene visualizzata una stampa)  */
int confronto_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indicce_porto);

/* stampa l'istantanea del mercato.
 * Usato in fase di debug ora da stampa_dump(). */
void stampa_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indice_porto);