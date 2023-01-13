typedef struct{
    int val;
    int exp;
} merce;

typedef struct {
    long mtype;
    int indicemerce;
    int nlotti;
}richiesta;


int shm_mercato(int par_SO_PORTI, int par_SO_MERCI);

void* indirizzoMercato();

int sganciaMercato();

int coda_richieste();

int inviaRichiesta(richiesta rich);

richiesta accettaRichiesta(int nporto);

merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA);

int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, merce (*ptr)[par_SO_MERCI], int i);

merce caricamerci(int indiceporto, int indicemerce, int nlotti, int pesolotto, int spaziolibero, int scadenza, int par_SO_MERCI);

int scaricamerce(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI);

int shm_lotti(int par_SO_MERCI);

void* indirizzoDettagliLotti();

int sganciaDettagliLotti();

int distruggiCoda();

int distruggiShmDettagliLotti();

int distruggiMercato();