#include "definitions.h"
/*
    DA FIXARE I PARAMETRI ZIOPERONE E RIGUARDARE SETUP

*/
//agomenti della ftok
#define ftok_arg1 "/master.c"
#define ftok_arg2 1

//range alto del random
#define max_merci_spawn_random 10

//parametro per abbassare il random se troppo alto
#define riducirandom 3

//valore per merci che sono a 0 o in domanda (quindi, che non scadono)
#define noscadenza 50



typedef struct{
    int val;
    int exp;
} merce;

/*
    definizione puntatore a vettori di merci
    merce (*ptr)[parametro[SO_MERCI]];
*/



//id shared memori mercato
int shmid;

int shm_mercato(int par_SO_PORTI, int par_SO_MERCI){

    key_t chiave;
    int shm_size = par_SO_PORTI * par_SO_MERCI * sizeof(merce);

    chiave = ftok(ftok_arg1, ftok_arg2);

    TEST_ERROR

    int smid = shmget(chiave, shm_size, IPC_CREAT | IPC_EXCL | 0666);

    TEST_ERROR

    return shmid;
}

int getIdMercato(){
    return shmid;
}

merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA){
    srand(time(NULL));
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
    int r, tmp;

    //per ogni merce, aggiungi in offerta o domanda un random r lotti
    for(int j=0;j<par_SO_MERCI;j++){
        r = rand() % max_merci_spawn_random + 0;
        tmp = dettagliLotti[j]->val * r;

        //se il peso generato supera il valore di toFill, cicla fino ad un valore inferiore
        while(tmp>toFill){
            r = r-riducirandom;
            tmp = dettagliLotti[j]->val * r;
        }
        //se la riduzione del peso scende sotto lo zero, si imposta la merce a 0
        if(tmp < 0){
            (*(ptr+i)+j)->val = 0;
            (*(ptr+i)+j)->val = noscadenza;
            tmp = 0;
        }else if(r&1){ //se il random generato è pari, si imposta la merce in domanda
            (*(ptr+i)+j)->val = -r;
            (*(ptr+i)+j)->exp = noscadenza;
        }else{         //altrimenti si imposta in offerta
            (*(ptr+i)+j)->val = r;
            (*(ptr+i)+j)->exp = dettagliLotti[j]->exp;
        }

        toFill-=tmp;

        //se sono state generate domande e offerte pari a toFill, tutte le altre merci si impostano a 0
        if(toFill == 0){
            for(int k=j;k<par_SO_MERCI;k++){
                (*(ptr+i)+k)->val = 0;
                (*(ptr+i)+k)->exp = noscadenza;
            }
        }
    
    }

    //se è stato assegnato un valore per ogni merce, viene incrementato di 1 la domanda o l'offerta per ogni merce
    //fino a raggiungere toFill
    while(toFill>0){
        
        for(int j=0;j<par_SO_MERCI;j++){
            
            //aumenta offerta
            if((*(ptr+i)+j)->val > 0){
                if(toFill > dettagliLotti[j]->val){
                    (*(ptr+i)+j)->val++;
                    toFill-=dettagliLotti[j]->val;
                }
            }

            //aumenta domanda
            if((*(ptr+i)+j)->val < 0){
                if(toFill > dettagliLotti[j]->val ){
                    (*(ptr+i)+j)->val--;
                    toFill-= dettaglioLotti[j]->val;
                }
            }
        }
    }
}




