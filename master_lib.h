#ifndef _MASTER_LIB_H
#define _MASTER_LIB_H
#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"

/* Funzione che pulisce i file txt usati per i log
	viene chiamata all'avvio di ogni simulazione
*/
void clearLog();

/* Esegue tutte le verifiche sui parametri della simulazione.
 *
 * RITORNA:
 * - 1: se tutti i parametri sono accettati. 
 * - 0: in caso di violazione dei vincoli del progetto (PORTI < 4 e/o NAVI < 1).
 * - -1: in caso di errori che renderebbero instabile la simulazione. (se SPEED == 0 => avremo divisioni per zero) 
 * - -2: in caso di errori e violazione dei vincoli. */
int controllo_parametri(int PARAMETRO[]);

/* Alloca le risorse IPC di memorie condivise e coda.
 * Vengono passate per referenza le varibili, per impostare gli id nel master. */
void alloca_id(int *id_shm_mercato, int *id_shm_dettagli_lotti, int *id_shm_posizioni_porti, int *id_shm_dump, int *id_coda_richieste, int PARAMETRO[]);

/* Genera un punto casuale nella mappa di lato 'lato', senza modificare il seed di rand. */
point generate_random_point_master(int lato);

/* Genera le posizioni dei porti in vptr_shm_posizioni. */
void generate_positions(double lato, point* posizioni_porti, int PORTI);

/* inizializza i valori del dump, tra cui i puntatori, chiamato dal master a inizio simulazione */
void inizializza_dump(void *vptr_shm_dump, int PARAMETRO[]);

/* Crea randomicamente i valori in tonnellate per ogni lotto e le ripettive scadenze. */
void setUpLotto(merce* ptr_dettagli_lotti, int PARAMETRO[]);

/* Ritorna 1 se considera uguali x e y, 0 altrimenti. 
 * NOTA: equals funziona con una certa TOLLERANZA (definita in definitions). */
int equals(double x, double y);

/* stampa l'istantanea del mercato.
 * Usato in fase di debug ora da stampa_dump(). */
void stampa_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indice_porto);

/* Controlla l'effettiva presenza di altre merci in richiesta o in offerta.
 * Qualora entrambe siano assenti, terminerà la simulazione. */
int controlla_mercato(void *vptr_shm_mercato, void *vptr_shm_dump, int PARAMETRO[]);

/* Controlla la scadenza delle merci in mercato allo scattare di ogni nuovo giorno. 
 * 
 * NOTA:
 * - Aggiorna il dump delle merci e dei porti ed elimina le merci scadute dalla shm mercato. 
 * - Fa uso del semaforo del dump in posizione 0. */
void controllo_scadenze_porti(merce *p_lotti, void *p_mercato, void *p_dump, int id_sem_dump, int PARAMETRO[]);

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

/* distruzione finale delle risorse 
 * ----------------------------------- */

/* Distrugge le risorse di cui sono passati gli id. (shm e coda) */
void distruggi_risorse(int id_mercato, int id_lotti, int id_posizioni, int id_dump, int id_coda);

/* Usato dal master alla fine, per distruggere i semafori della simulazione. */
void distruggi_semafori(int id_sem_mercato, int id_sem_dump, int id_sem_banchine, int id_sem_gestione);

/* Esegue le free() necessarie a fine simulazione. */
void free_ptrs(int *childs, char **argv_figli, char **argv_demone, char **argv_meteo);
#endif