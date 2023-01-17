#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif

/* POSIZIONI ----------------------------------------- */

/* crea la memoria POSIZIONI, ritorna errno */
int alloca_shm_posizioni(int PORTI);

/* Ritorna l'indirizzo di POSIZIONI */
void *indirizzoPosizioni();

/* Sgancia POSIZIONI */
int sgancia_shm_posizioni();

/* Distrugge POSIZIONI */
int distruggi_shm_posizioni();

/* DUMP -------------------------------------------------- */

/* TRASFERIRE QUA I METODI CHE RIGUARDANO IL DUMP ! */










/* LOTTI ----------------------------------------------- */

/* Crea la memoria LOTTI, ritorna errno */
int alloca_shm_lotti(int par_SO_MERCI);

/* Ritorna l'indirizzo di LOTTI */
void *indirizzoDettagliLotti();

/* Sgancia LOTTI */
int sgancia_shm_dettagliLotti();

/* Distrugge LOTTI */
int distruggi_shm_dettagliLotti();

/* LOTTI: Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA);

/* SET-UP DI MERCATO */
int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, merce (*ptr)[par_SO_MERCI], int i);

/* CODA RICHIESTE ----------------------------------------*/

/* Crea la coda RICHIESTE, ne ritorna l'id */
int crea_coda_richieste();

/* Distrugge coda RICHIESTE */
int distruggi_coda_richieste();

/* Invia una richiesta alla coda */
int inviaRichiesta(richiesta rich);

/* Accetta una richiesta alla coda */
richiesta accettaRichiesta(int nporto);

/* MERCATO --------------------------------------------- */

/* Crea memoria MERCATO, ritorna errno */
int alloca_shm_mercato(int par_SO_PORTI, int par_SO_MERCI);

/* Ritorna l'indirizzo di MERCATO */
void *indirizzoMercato();

/* Sgancia MERCATO */
int sgancia_shm_mercato();

/* Distrugge MERCATO */
int distruggi_shm_mercato();

/* MERCATO: */
merce caricamerci(int indiceporto, int indicemerce, int nlotti, int pesolotto, int spaziolibero, int scadenza, int par_SO_MERCI);

/* MERCATO: */
int scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI);



