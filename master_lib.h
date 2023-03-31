#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#ifndef _SEM_LIB_H
	#include "sem_lib.h"
#endif
/* Funzione che pulisce i file txt usati per i log
*/
void clearLog();
 
/* Funzione d'ausilio per controllare che sia corretto il dump:
 * - Controlla che mercato_shm sia uguale a dump_shm
 * RITORNA: 1 se è tutto ok, 0 altrimenti. (viene visualizzata una stampa)  */
int confronto_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indicce_porto);

/* stampa l'istantanea del mercato.
 * Usato in fase di debug ora da stampa_dump(). */
void stampa_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indice_porto);

/* Controlla l'effettiva presenza di altre merci in richiesta o in offerta.
 * Qualora entrambe siano assenti, terminerà la simulazione. */
int controlla_mercato(void *vptr_shm_mercato, void *vptr_shm_dump, int PARAMETRO[]);

/* Stampa in ordine:
 * - le merci, coi 5 parametri richiesti
 * - i porti, coi 3 parametri richiesti + le banchine [occupate/presenti]
 * - le navi, coi 3 parametri richiesti
 * CHIAMATO da stampa_dump() e stampa_terminazione(). */
void stampa_merci_porti_navi(int PARAMETRO[], void * vptr_shm_dump, void *vptr_shm_mercato, int id_semaforo_banchine);

/* Stampa il dump in quel determinato momento. */
void stampa_dump(int PARAMETRO[], void * vptr_shm_dump, void * vptr_shm_mercato, int id_semaforo_banchine);

/* Calcola il porto con più spedizioni e quello con più ricezioni. */
void calcola_porti_term(int PARAMETRO[], void* vptr_shm_dump);

/* Stampa il dump alla terminazione della simulazione. */
void stampa_terminazione(int PARAMETRO[], void * vptr_shm_dump, void * vptr_shm_mercato, int id_semaforo_banchine);

/* Esegue le free() necessarie a fine simulazione; size è la lunghezza di argv_figli. */
void free_ptr(void *childs, void* argv_figli[], int size);