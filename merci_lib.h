#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif

/* Range alto del random */
#define MAX_MERCI_SPAWN_RANDOM 10000

/* Parametro per abbassare il random, se troppo alto */
#define RIDUCI_RANDOM 30

/* Valore per merci che sono a 0 o in domanda (quindi, che non scadono) */
#define NO_SCADENZA 50

/* id queue richieste */
static int id_coda_richieste;

/* Aggancia SHM */
void* aggancia_shm(int id);

/* Sgancia SHM */
int sgancia_shm(void *ptr);

/* Distrugge SHM */
int distruggi_shm(int id);

/* POSIZIONI ----------------------------------------- */

/* crea la memoria POSIZIONI, ritorna errno */
int alloca_shm_posizioni(int PORTI);

/* fa la get senza creare */
int set_shm_posizioni(int PORTI);

/* LOTTI ----------------------------------------------- */

/* Crea la memoria LOTTI, ritorna errno */
int alloca_shm_lotti(int par_SO_MERCI);

/* fa la get senza creare */
int set_shm_lotti(int par_SO_MERCI);

/* LOTTI: Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA);

/* SET-UP DI MERCATO */
int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, int i, void* ptr_shm_mercato, void* ptr_shm_dettaglilotti);

/* MERCATO --------------------------------------------- */

/* Crea memoria MERCATO, ritorna errno */
int alloca_shm_mercato(int par_SO_PORTI, int par_SO_MERCI);

/* fa la get senza creare */
int set_shm_mercato(int par_SO_PORTI, int par_SO_MERCI);

/* MERCATO: */
merce caricamerci(richiesta rkst, int spaziolibero, int scadenza, int par_SO_MERCI,  void *ptr_mercato, merce* ptr_lotti);
/* MERCATO: */
int scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI, void *ptr_mercato);

/* CODA RICHIESTE ----------------------------------------*/

/* Crea la coda RICHIESTE, ne ritorna l'id */
int alloca_coda_richieste();

/* fa la get senza creare */
int set_coda_richieste();

/* Ritorna l'id della coda.
 * (va prima settato il valore statico) */
int getIdCoda();

/* Distrugge coda RICHIESTE */
int distruggi_coda();

/* Invia una richiesta alla coda */
int inviaRichiesta(richiesta rich);

/* Accetta una richiesta alla coda */
richiesta accettaRichiesta(int nporto);