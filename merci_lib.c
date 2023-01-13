#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* agomenti della ftok */
#define ftok_arg1 "/master.c"
#define ftok_arg2 1

/* range alto del random */
#define MAX_MERCI_SPAWN_RANDOM 10

/* parametro per abbassare il random se troppo alto */
#define RIDUCI_RANDOM 3

/* valore per merci che sono a 0 o in domanda (quindi, che non scadono) */
#define NO_SCADENZA 50

/*
    definizione puntatore a vettori di merci
    merce (*ptr)[parametro[SO_MERCI]];
*/

/* id shared memori mercato */
int shmmercato;
int qid;
int shmlotti;
 
 /*puntatore al mercato*/
merce(*puntatore)[];
merce* dettagliLotti;

/* Crea memoria mercato */
int shm_mercato(int par_SO_PORTI, int par_SO_MERCI){
    key_t chiave;
    int shm_size = par_SO_PORTI * par_SO_MERCI * sizeof(merce);

    chiave = KEY_MERCATO;
    TEST_ERROR

    shmmercato = shmget(chiave, shm_size, IPC_CREAT | IPC_EXCL | 0666);

    TEST_ERROR
    
    puntatore = shmat(shmmercato, NULL, 0);

    TEST_ERROR

    return 1;
}

/* crea la memoria lotti*/
int shm_lotti(int par_SO_MERCI){  
    shmlotti = shmget(KEY_LOTTI, (sizeof(merce)*par_SO_MERCI), IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR

    dettagliLotti = shmat(shmlotti, NULL, 0);

    TEST_ERROR

    return 1;
}

void* indirizzoDettagliLotti(){
    return dettagliLotti;
}

int sganciaDettagliLotti(){
    return shmdt(dettagliLotti);
}

void* indirizzoMercato(){
    return puntatore;
}

int sganciaMercato(){
    return shmdt(puntatore);
}

/* crea la coda richieste */
int coda_richieste(){
    
    qid = msgget(KEY_CODA_RICHIESTE, IPC_CREAT | IPC_EXCL | 0666);

    TEST_ERROR;
    
    return qid;
    
}

int inviaRichiesta(richiesta rich){

    msgsnd(qid, &rich, SIZE_MSG, 0);
    TEST_ERROR

    return 1;
}

richiesta accettaRichiesta(int nporto){
    richiesta ritorno;

    if(msgrcv(qid, &ritorno, SIZE_MSG, nporto, IPC_NOWAIT)<0){
        ritorno.indicemerce-1;
    }
    TEST_ERROR

    return ritorno;
}

int distruggiCoda(){
    msgctl(qid, IPC_RMID, 0);

    TEST_ERROR

    return 1;
}

int distruggiMercato(){
    shmctl(shmmercato, IPC_RMID, 0);

    TEST_ERROR

    return 1;
}

int distruggiShmDettagliLotti(){
    shmctl(shmlotti, IPC_RMID, 0);

    TEST_ERROR

    return 1;
}

/* Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA){
    /* il chiamante deve settare un random */
    static int lottoDaUno = 0;
    static int mlette = 0;
    merce tmp;
    tmp.val = rand() % par_SO_SIZE + 1;
    tmp.exp = par_SO_MIN_VITA + rand() % (par_SO_MAX_VITA - par_SO_MIN_VITA + 1);
    if(tmp.val == 1){
        lottoDaUno = 1;
    }
    if(mlette == (nmerci-1) && lottoDaUno == 0){
        tmp.val = 1;
    }
    return tmp;
}


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