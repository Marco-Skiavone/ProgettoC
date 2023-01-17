#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "merci_lib.h"

/* Range alto del random */
#define MAX_MERCI_SPAWN_RANDOM 10

/* Parametro per abbassare il random, se troppo alto */
#define RIDUCI_RANDOM 3

/* Valore per merci che sono a 0 o in domanda (quindi, che non scadono) */
#define NO_SCADENZA 50

/*
    definizione puntatore a vettori di merci
    merce (*ptr)[parametro[SO_MERCI]];
*/


/* id shared memory mercato */
static int shmmercato;
/* id queue richieste */
static int qid;
/* id shared memory lotti */
static int shmlotti;
/* id shared memory posizioni */
static int posizioni_id;

/* Puntatore alla memoria mercato */
static merce *puntatore;
/* Puntatore alla memoria lotti */
static merce *dettagliLotti;
/* Puntatore alla memoria posizioni */
static point *posizioni_ptr;


/* POSIZIONI ------------------------- */

/* crea la memoria POSIZIONI, ritorna errno */
int shm_posizioni(int PORTI){
    int size_posizioni = sizeof(point)*PORTI;
    posizioni_id = shmget(KEY_POSIZIONI, size_posizioni, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    posizioni_ptr = shmat(posizioni_id, NULL, 0);
    return errno;
}   

/* Ritorna l'indirizzo di POSIZIONI */
void *indirizzoPosizioni(){
    return posizioni_ptr;
}
 /* Aggiungi la shm di POSIZIONI alla memoria del processo*/
void* agganciaPosizioni(){
    void* ret_val = shmat(posizioni_id, NULL, 0);
    TEST_ERROR
    return ret_val;
}

/* Sgancia POSIZIONI */
int sganciaPosizioni(){
    int ret_val = shmdt(posizioni_ptr);
    TEST_ERROR
    return ret_val;
}

/* Distrugge POSIZIONI */
int distruggiPosizioni(){
    int return_val = shmctl(posizioni_id, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}


/* DUMP ---------------------------------------------------------- */

/* TRASFERIRE QUA LE FUNZIONI PER IL DUMP*/



/* LOTTI ----------------------------------------------- */

/* Crea la memoria LOTTI, ritorna errno */
int shm_lotti(int par_SO_MERCI){  
    shmlotti = shmget(KEY_LOTTI, (sizeof(merce)*par_SO_MERCI), IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    dettagliLotti = shmat(shmlotti, NULL, 0);
    TEST_ERROR
    return shmlotti;
}

/* Ritorna l'indirizzo di LOTTI */
void *indirizzoDettagliLotti(){
    return dettagliLotti;
}

 /* Aggancia lo spazio dei LOTTI alla memoria del processo*/
void* agganciaDettaglioLott(){
    void* ret_val = shmat(shm_lotti, NULL, 0);
    TEST_ERROR
    return ret_val;
} 

/* Sgancia LOTTI */
int sganciaDettagliLotti(){
    int return_val = shmdt(dettagliLotti);
    TEST_ERROR
    return return_val;
}

/* Distrugge LOTTI */
int distruggiShmDettagliLotti(){
    int return_val = shmctl(shmlotti, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}

/* LOTTI: Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA){
    /* il chiamante deve settare un random */
    static int lottoDaUno = 0;
    static int mlette = 0;
    merce tmp;
    tmp.val = (rand() % par_SO_SIZE) + 1;
    tmp.exp = par_SO_MIN_VITA + (rand() % (par_SO_MAX_VITA - par_SO_MIN_VITA));
    if(tmp.val == 1){
        lottoDaUno = 1;
    }
    if(mlette == (nmerci-1) && lottoDaUno == 0){
        tmp.val = 1;
    }
    return tmp;
}

/* CODA RICHIESTE ----------------------------------------*/

/* Crea la coda RICHIESTE, ne ritorna l'id */
int coda_richieste(){
    qid = msgget(KEY_CODA_RICHIESTE, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR;
    
    return qid;
}

/* Distrugge coda RICHIESTE */
int distruggiCoda(){
    int return_val = msgctl(qid, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}

/* Invia una richiesta alla coda */
int inviaRichiesta(richiesta rich){
    int ret_val = msgsnd(qid, &rich, SIZE_MSG, 0);
    TEST_ERROR
    return ret_val;
}

/* Accetta una richiesta alla coda */
richiesta accettaRichiesta(int nporto){
    richiesta ritorno;
    if(msgrcv(qid, &ritorno, SIZE_MSG, nporto, IPC_NOWAIT) < 0){
        ritorno.indicemerce-1;
    }
    TEST_ERROR
    return ritorno;
}

/* MERCATO -------------------------------------------------- */

/* Crea memoria MERCATO, ritorna shmmercato */
int shm_mercato(int par_SO_PORTI, int par_SO_MERCI){
    key_t chiave = KEY_MERCATO;
    int shm_size = par_SO_PORTI * par_SO_MERCI * sizeof(merce);
    TEST_ERROR

    shmmercato = shmget(chiave, shm_size, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    
    puntatore = shmat(shmmercato, NULL, 0);
    TEST_ERROR
    return shmmercato;
}

/* Ritorna l'indirizzo di MERCATO */
void *indirizzoMercato(){
    return puntatore;
}

/* Aggancia MERCATO*/
void* agganciaMercat(){
    void* ret_val = shmat(shm_mercato, NULL, 0);
    TEST_ERROR
    return ret_val;
}

/* Sgancia MERCATO */
int sganciaMercato(){
    int ret_val = shmdt(puntatore);
    TEST_ERROR
    return ret_val;
}

/* Distrugge MERCATO */
int distruggiMercato(){
    int return_val = shmctl(shmmercato, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}

/* SET-UP DI MERCATO */
int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, merce (*ptr)[par_SO_MERCI], int i){
    int toFill = par_SO_FILL;
    int j, k, r, tmp;

    /* per ogni merce, aggiungi in offerta o domanda un random r lotti */
    for(j=0;j<par_SO_MERCI;j++){
        r = rand() % MAX_MERCI_SPAWN_RANDOM + 0;
        tmp = dettagliLotti[j].val * r;

        /* se il peso generato supera il valore di toFill, cicla fino ad un valore inferiore */
        while(tmp>toFill){
            r = r-RIDUCI_RANDOM;
            tmp = dettagliLotti[j].val * r;
        }
        /* se la riduzione del peso scende sotto lo zero, si imposta la merce a 0 */
        if(tmp < 0){
            (*(ptr+i)+j)->val = 0;
            (*(ptr+i)+j)->val = NO_SCADENZA;
            tmp = 0;
        }else if(r&1){ /* se il random generato è pari, si imposta la merce in domanda */
            (*(ptr+i)+j)->val = -r;
            (*(ptr+i)+j)->exp = NO_SCADENZA;
        }else{         /* altrimenti si imposta in offerta */
            (*(ptr+i)+j)->val = r;
            (*(ptr+i)+j)->exp = dettagliLotti[j].exp;
        }

        toFill-=tmp;

        /* se sono state generate domande e offerte pari a toFill, tutte le altre merci si impostano a 0 */
        if(toFill == 0){
            for(k=j;k<par_SO_MERCI;k++){
                (*(ptr+i)+k)->val = 0;
                (*(ptr+i)+k)->exp = NO_SCADENZA;
            }
        }
    
    }

    /* se è stato assegnato un valore per ogni merce, viene incrementato di 1 la domanda o l'offerta per ogni merce
    fino a raggiungere toFill */
    while(toFill>0){
        
        for(j=0;j<par_SO_MERCI;j++){
            
            /* aumenta offerta */
            if((*(ptr+i)+j)->val > 0){
                if(toFill > dettagliLotti[j].val){
                    (*(ptr+i)+j)->val++;
                    toFill-=dettagliLotti[j].val;
                }
            }

            /* aumenta domanda */
            if((*(ptr+i)+j)->val < 0){
                if(toFill > dettagliLotti[j].val ){
                    (*(ptr+i)+j)->val--;
                    toFill-= dettagliLotti[j].val;
                }
            }
        }
    }
}

/*
int caricamerci(int indiceporto, int indicemerce, int nlotti, int par_SO_MERCI){
    merce (*ptr)[par_SO_MERCI] = agganciaMercato();
    int exp;
    (*(ptr+indicemerce)+indicemerce)->val -= nlotti;
    if((*(ptr+indicemerce)+indicemerce)->val == 0){
        
    }
}
*/

/* MERCATO: */
merce caricamerci(int indiceporto, int indicemerce, int nlotti, int pesolotto, int spaziolibero, int scadenza, int par_SO_MERCI){
    merce (*ptr)[par_SO_MERCI] = indirizzoMercato();
    merce ritorno;

    while(nlotti * pesolotto > spaziolibero){
        nlotti--;
    }

    if((*(ptr+indiceporto)+indicemerce)->exp<scadenza){
        ritorno.val = -1;
        ritorno.exp = -1;
        return ritorno;
    }

    if((*(ptr+indiceporto)+indicemerce)->val > nlotti){
        ritorno.val -= nlotti;
        ritorno.exp = (*(ptr+indiceporto)+indicemerce)->val;

    }else if((*(ptr+indiceporto)+indicemerce)->val == nlotti){
        ritorno.val = nlotti;
        ritorno.exp = (*(ptr+indiceporto)+indicemerce)->exp;
        (*(ptr+indiceporto)+indicemerce)->val = 0;
        (*(ptr+indiceporto)+indicemerce)->exp = NO_SCADENZA;

    }else{
        ritorno.val = (*(ptr+indiceporto)+indicemerce)->val;
        ritorno.exp = (*(ptr+indiceporto)+indicemerce)->exp;
        (*(ptr+indiceporto)+indicemerce)->val = 0;
        (*(ptr+indiceporto)+indicemerce)->exp = NO_SCADENZA;
    }

    return ritorno;
}

/* MERCATO: */
int scaricamerce(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI){
    merce (*ptr)[par_SO_MERCI] = indirizzoMercato();
    if(scarico.exp >= data){
        
        (*(ptr+indiceporto)+indicemerce)->val += scarico.val;
        (*(ptr+indiceporto)+indicemerce)->exp = NO_SCADENZA;

        /*
            aggiorna consegnata al porto
        */
        
        return 1;
    }else{

        /*
            aggiorna scaduta in mare;
        */
       return 0;
    }
}