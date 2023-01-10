#include "definitions.h"

#define TEST_ERROR 							\
	if (errno) {							\
		fprintf(stderr,						\
		"%s:%d: PID=%5d: Error %d (%s)\n", 	\
		__FILE__,							\
		__LINE__,							\
		getpid(),							\
		errno,								\
		strerror(errno));					\
	}

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
    merce (*ptr)[SO_MERCI];
*/

merce dettagliLotti[SO_MERCI];

//id shared memori mercato
int shmid;

int shm_mercato(){

    key_t chiave;
    int shm_size = SO_PORTI * SO_MERCI * sizeof(merce);

    chiave = ftok(ftok_arg1, ftok_arg2);

    TEST_ERROR

    int smid = shmget(chiave, shm_size, IPC_CREAT | IPC_EXCL | 0666);

    TEST_ERROR

    return shmid;
}

int getIdMercato(){
    return shmid;
}

int setUpLotti(){
    srand(time(NULL));
    int lottoDaUno = 0;

    //setuppa il vettore delle merci con le informazioni sul peso dei lotti e sulla scadenza
    for(int i=0;i<SO_MERCI;i++){
        dettagliLotti[i].val = rand() % SO_SIZE + 1;
        dettagliLotti[i].exp = SO_MIN_VITA + rand() % (SO_MAX_VITA - SO_MIN_VITA + 1);
        //controlla se esiste un lotto da 1 tonnellata
        if(dettagliLotti[i].val == 0){  lottoDaUno = 1; }
    }
    //setta lotto da 1 tonnellata se non esiste
    if(!lottoDaUno){
        dettagliLotti[rand()% SO_MERCI + 0].val = 1;
    }

}


int spawnMerciPorti(merce (*ptr)[SO_PORTI], int i){
    int toFill = SO_FILL;
    int r, tmp;

    //per ogni merce, aggiungi in offerta o domanda un random r lotti
    for(int j=0;j<SO_MERCI;j++){
        r = rand() % max_merci_spawn_random + 0;
        tmp = dettagliLotti[j].val * r;

        //se il peso generato supera il valore di toFill, cicla fino ad un valore inferiore
        while(tmp>toFill){
            r = r-riducirandom;
            tmp = dettagliLotti[j].val *r;
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
            (*(ptr+i)+j)->exp = dettagliLotti[j].exp;
        }

        toFill-=tmp;

        //se sono state generate domande e offerte pari a toFill, tutte le altre merci si impostano a 0
        if(toFill == 0){
            for(int k=j;k<SO_MERCI;k++){
                (*(ptr+i)+k)->val = 0;
                (*(ptr+i)+k)->exp = noscadenza;
            }
        }
    
    }

    //se è stato assegnato un valore per ogni merce, viene incrementato di 1 la domanda o l'offerta per ogni merce
    //fino a raggiungere toFill
    while(toFill>0){
        
        for(int j=0;j<SO_MERCI;j++){
            
            //aumenta offerta
            if((*(ptr+i)+j)->val > 0){
                if(toFill > dettagliLotti[j].val){
                    (*(ptr+i)+j)->val++;
                    toFill-=dettagliLotti[j].val;
                }
            }

            //aumenta domanda
            if((*(ptr+i)+j)->val < 0){
                if(toFill > dettagliLotti[j].val){
                    (*(ptr+i)+j)->val--;
                    toFill-=dettagliLotti[j].val;
                }
            }
        }
    }
}




