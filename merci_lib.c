#include "definitions.h"
/*
    DA FIXARE I PARAMETRI ZIOPERONE E RIGUARDARE SETUP

*/
/* agomenti della ftok */
#define ftok_arg1 "/master.c"
#define ftok_arg2 1

/* range alto del random */
#define max_merci_spawn_random 10

/* parametro per abbassare il random se troppo alto */
#define riducirandom 3

/* valore per merci che sono a 0 o in domanda (quindi, che non scadono) */
#define noscadenza 50



#define msgsize sizeof(int)*2


typedef struct {
    int val;
    int exp;
} merce;

typedef struct {
    long mtype;
    int indicemerce;
    int nlotti;
}richiesta;

/*
    definizione puntatore a vettori di merci
    merce (*ptr)[parametro[SO_MERCI]];
*/



/* id shared memori mercato */
int shmid;
int qid;

merce(*puntatore)[];

int shm_mercato(int par_SO_PORTI, int par_SO_MERCI){

    key_t chiave;
    int shm_size = par_SO_PORTI * par_SO_MERCI * sizeof(merce);

    chiave = KEY_MERCATO;
    TEST_ERROR

    shmid = shmget(chiave, shm_size, IPC_CREAT | IPC_EXCL | 0666);

    TEST_ERROR
    
    puntatore = shmat(shmid, NULL, 0);

    TEST_ERROR

    return 1;
}

void* indirizzoMercato(){
    return puntatore;
}

int sganciaMercato(){
    return shmdt(puntatore);
}


int coda_richieste(){
    int esito;
    qid = KEY_CODA_RICHIESTE;
    esito = msgget(qid, IPC_CREAT | IPC_EXCL | 0666);

    TEST_ERROR;
    
    return esito;
    
}

int inviaRichiesta(richiesta rich){

    msgsnd(qid, &rich, msgsize, 0);
    TEST_ERROR

    return 1;
}

richiesta accettaRichiesta(int nporto){
    richiesta ritorno;

    if(msgrcv(qid, &ritorno, msgsize, nporto, IPC_NOWAIT)<0){
        ritorno.indicemerce-1;
    }
    TEST_ERROR

    return ritorno;
}


merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA){
    /* il chiamante deve settare un random */
    static int lottoDaUno = 0;
    static int mlette = 0;
    merce tmp;
    tmp.val = rand() % par_SO_SIZE + 1;
    tmp.exp = par_SO_MIN_VITA+ rand() % (par_SO_MAX_VITA - par_SO_MAX_VITA + 1);
    if(tmp.val == 1){
        lottoDaUno = 1;
    }
    if(mlette == (nmerci-1) && lottoDaUno == 0){
        tmp.val = 1;
    }
    return tmp;
}


int spawnMerciPorti(int par_SO_FILL, int par_SO_PORTI, int par_SO_MERCI, merce (*ptr)[par_SO_MERCI], int i, merce (*dettagliLotti)[par_SO_MERCI]){
    int toFill = par_SO_FILL;
    int j, k, r, tmp;

    /* per ogni merce, aggiungi in offerta o domanda un random r lotti */
    for(j=0;j<par_SO_MERCI;j++){
        r = rand() % max_merci_spawn_random + 0;
        tmp = dettagliLotti[j]->val * r;

        /* se il peso generato supera il valore di toFill, cicla fino ad un valore inferiore */
        while(tmp>toFill){
            r = r-riducirandom;
            tmp = dettagliLotti[j]->val * r;
        }
        /* se la riduzione del peso scende sotto lo zero, si imposta la merce a 0 */
        if(tmp < 0){
            (*(ptr+i)+j)->val = 0;
            (*(ptr+i)+j)->val = noscadenza;
            tmp = 0;
        }else if(r&1){ /* se il random generato è pari, si imposta la merce in domanda */
            (*(ptr+i)+j)->val = -r;
            (*(ptr+i)+j)->exp = noscadenza;
        }else{         /* altrimenti si imposta in offerta */
            (*(ptr+i)+j)->val = r;
            (*(ptr+i)+j)->exp = dettagliLotti[j]->exp;
        }

        toFill-=tmp;

        /* se sono state generate domande e offerte pari a toFill, tutte le altre merci si impostano a 0 */
        if(toFill == 0){
            for(k=j;k<par_SO_MERCI;k++){
                (*(ptr+i)+k)->val = 0;
                (*(ptr+i)+k)->exp = noscadenza;
            }
        }
    
    }

    /* se è stato assegnato un valore per ogni merce, viene incrementato di 1 la domanda o l'offerta per ogni merce
    fino a raggiungere toFill */
    while(toFill>0){
        
        for(j=0;j<par_SO_MERCI;j++){
            
            /* aumenta offerta */
            if((*(ptr+i)+j)->val > 0){
                if(toFill > dettagliLotti[j]->val){
                    (*(ptr+i)+j)->val++;
                    toFill-=dettagliLotti[j]->val;
                }
            }

            /* aumenta domanda */
            if((*(ptr+i)+j)->val < 0){
                if(toFill > dettagliLotti[j]->val ){
                    (*(ptr+i)+j)->val--;
                    toFill-= dettagliLotti[j]->val;
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
        (*(ptr+indiceporto)+indicemerce)->exp = noscadenza;

    }else{
        ritorno.val = (*(ptr+indiceporto)+indicemerce)->val;
        ritorno.exp = (*(ptr+indiceporto)+indicemerce)->exp;
        (*(ptr+indiceporto)+indicemerce)->val = 0;
        (*(ptr+indiceporto)+indicemerce)->exp = noscadenza;
    }

    return ritorno;
}

int scaricamerce(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI){
    merce (*ptr)[par_SO_MERCI] = indirizzoMercato();
    if(scarico.exp >= data){
        
        (*(ptr+indiceporto)+indicemerce)->val += scarico.val;
        (*(ptr+indiceporto)+indicemerce)->exp = noscadenza;

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