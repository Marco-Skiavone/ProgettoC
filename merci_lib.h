#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif

/* POSIZIONI ----------------------------------------- */

/* crea la memoria POSIZIONI, ritorna errno */
int shm_posizioni(int PORTI);

/* Ritorna l'indirizzo di POSIZIONI */
void *indirizzoPosizioni();

/* Sgancia POSIZIONI */
int sganciaPosizioni();

/* Distrugge POSIZIONI */
int distruggiPosizioni();

/* DUMP -------------------------------------------------- */

/* TRASFERIRE QUA I METODI CHE RIGUARDANO IL DUMP ! */










/* LOTTI ----------------------------------------------- */

/* Crea la memoria LOTTI, ritorna errno */
int shm_lotti(int par_SO_MERCI);

/* Ritorna l'indirizzo di LOTTI */
void *indirizzoDettagliLotti();

/* Sgancia LOTTI */
int sganciaDettagliLotti();

/* Distrugge LOTTI */
int distruggiShmDettagliLotti();

/* LOTTI: Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA);

/* SET-UP DI MERCATO */
int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, merce *ptr[], int i);

/* CODA RICHIESTE ----------------------------------------*/

/* Crea la coda RICHIESTE, ne ritorna l'id */
int coda_richieste();

/* Distrugge coda RICHIESTE */
int distruggiCoda();

/* Invia una richiesta alla coda */
int inviaRichiesta(richiesta rich);

/* Accetta una richiesta alla coda */
richiesta accettaRichiesta(int nporto);

/* MERCATO --------------------------------------------- */

/* Crea memoria MERCATO, ritorna errno */
int shm_mercato(int par_SO_PORTI, int par_SO_MERCI);

/* Ritorna l'indirizzo di MERCATO */
void *indirizzoMercato();

/* Sgancia MERCATO */
int sganciaMercato();

/* Distrugge MERCATO */
int distruggiMercato();

/* MERCATO: */
merce caricamerci(int indiceporto, int indicemerce, int nlotti, int pesolotto, int spaziolibero, int scadenza, int par_SO_MERCI);

/* MERCATO: */
int scaricamerce(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI);



