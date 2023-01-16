#ifndef _DEFINITIONS_H
    #include "definitions.h"
#endif

/* Crea memoria MERCATO, ritorna errno */
int shm_mercato(int par_SO_PORTI, int par_SO_MERCI);

void *indirizzoMercato();

int sganciaMercato();

/* Crea la coda richieste, ne ritorna l'id */
int coda_richieste();

int inviaRichiesta(richiesta rich);

richiesta accettaRichiesta(int nporto);

/* Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA);

int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, merce (*ptr)[par_SO_MERCI], int i);

merce caricamerci(int indiceporto, int indicemerce, int nlotti, int pesolotto, int spaziolibero, int scadenza, int par_SO_MERCI);

int scaricamerce(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI);

/* crea la memoria LOTTI, ritorna errno */
int shm_lotti(int par_SO_MERCI);

/* Ritorna l'indirizzo di LOTTI */
void *indirizzoDettagliLotti();

/* Sgancia la memoria LOTTI */
int sganciaDettagliLotti();

/* crea la memoria POSIZIONI, ritorna errno */
int shm_posizioni(int PORTI);

/* Ritorna l'indirizzo di POSIZIONI */
void *indirizzoPosizioni();

int sganciaPosizioni();

int distruggiCoda();

int distruggiPosizioni();

int distruggiShmDettagliLotti();

int distruggiMercato();