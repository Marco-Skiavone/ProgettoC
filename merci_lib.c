#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "merci_lib.h"

/*
    definizione ptr_shm_mercato a vettori di merci
    merce (*ptr)[parametro[SO_MERCI]];
*/

/* alloca -> crea 
 * set -> fa get e non crea
 * get -> ritorna i valori senza fare delle _get 
 */

/* id queue richieste */
static int id_coda_richieste;

/* Ritorna l'indirizzo della shm */
void *aggancia_shm(int id){
    void * ret_val = shmat(id, NULL, 0);
    TEST_ERROR
    return ret_val;
}

/* Sgancia una SHM */
int sgancia_shm(void *ptr){
    int ret_val = shmdt(ptr);
    TEST_ERROR
    return ret_val;
}

/* Distrugge una SHM */
int distruggi_shm(int id){
    int return_val = shmctl(id, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}

/* POSIZIONI ------------------------- */

/* crea la memoria POSIZIONI, ritorna errno */
int alloca_shm_posizioni(int PORTI){
    int id = shmget(KEY_POSIZIONI, sizeof(point)*PORTI, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    return id;
}

/* fa la get senza creare */
int set_shm_posizioni(int PORTI){
    int size_posizioni = sizeof(point)*PORTI;
    int id = shmget(KEY_POSIZIONI, size_posizioni, PERMESSI);
    /*printf("id_posizioni dopo settaggio: %d\n", id);*/
    TEST_ERROR
    return id;
}

/* LOTTI ----------------------------------------------- */

/* Crea la memoria LOTTI, ritorna errno */
int alloca_shm_lotti(int par_SO_MERCI){  
    int id = shmget(KEY_LOTTI, (sizeof(merce)*par_SO_MERCI), IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    return id;
}

/* fa la get senza creare */
int set_shm_lotti(int par_SO_MERCI){
    int id = shmget(KEY_LOTTI, (sizeof(merce)*par_SO_MERCI), PERMESSI);
    TEST_ERROR
    return id;
}

/* LOTTI: Il chiamante deve settare un random */
merce setUpLotto(int nmerci, int par_SO_SIZE, int par_SO_MIN_VITA, int par_SO_MAX_VITA){
    /* il chiamante deve settare un random */
    static int lottoDaUno = 0;
    merce tmp;
    tmp.val = (rand() % par_SO_SIZE) + 1;
    tmp.exp = par_SO_MIN_VITA + (rand() % (par_SO_MAX_VITA - par_SO_MIN_VITA));
    if(!lottoDaUno){
        tmp.val = 1;
        lottoDaUno = 1;
    }
    return tmp;
}

/* CODA RICHIESTE ----------------------------------------*/

/* Crea la coda RICHIESTE, ne ritorna l'id */
int alloca_coda_richieste(){
    id_coda_richieste = msgget(KEY_CODA_RICHIESTE, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR;
    return id_coda_richieste;
}

/* fa la get di richieste senza creare */
int set_coda_richieste(){
    id_coda_richieste = msgget(KEY_CODA_RICHIESTE, PERMESSI);
    /*printf("id_coda: %d\n", id_coda_richieste);*/
    TEST_ERROR;
    return id_coda_richieste;
}

/* Ritorna l'id della coda.
 * (va prima settato il valore statico) */
int getIdCoda(){
    return id_coda_richieste;
}

/* Distrugge coda RICHIESTE */
int distruggi_coda(){
    int return_val = msgctl(id_coda_richieste, IPC_RMID, NULL);
    TEST_ERROR
    return return_val;
}

/* Invia una richiesta alla coda */
int inviaRichiesta(richiesta rich){
    rich.mtype += 1;
    int ret_val = msgsnd(id_coda_richieste, &rich, SIZE_MSG, 0);
    TEST_ERROR
    return ret_val;
}

/* Accetta una richiesta alla coda di tipo nporto(+1) */
richiesta accettaRichiesta(int nporto){
    richiesta ritorno;
    if(msgrcv(id_coda_richieste, &ritorno, SIZE_MSG, nporto+1, IPC_NOWAIT) < 0){
        ritorno.mtext.indicemerce = -1;
    }
    if(errno==42){
        errno = 0;
    }
    TEST_ERROR
    ritorno.mtype -=1;
    return ritorno;
}

/* MERCATO -------------------------------------------------- */

/* Crea memoria MERCATO, ritorna id_shm_mercato */
int alloca_shm_mercato(int par_SO_PORTI, int par_SO_MERCI){
    int shm_size = par_SO_PORTI * par_SO_MERCI * sizeof(merce);
    int id = shmget(KEY_MERCATO, shm_size, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    return id;
}

/* fa la get senza creare */
int set_shm_mercato(int par_SO_PORTI, int par_SO_MERCI){
    int shm_size = par_SO_PORTI * par_SO_MERCI * sizeof(merce);
    int id = shmget(KEY_MERCATO, shm_size, PERMESSI);
    TEST_ERROR
    return id;
}

/* SET-UP DI MERCATO */
int spawnMerciPorti(int par_SO_FILL, int par_SO_MERCI, int i, void* ptr_shm_mercato, void* ptr_shm_dettaglilotti){
    int toFill = par_SO_FILL;
    int j, k, r, tmp;
    merce (*ptr)[par_SO_MERCI] = (merce(*)[par_SO_MERCI])ptr_shm_mercato;
    merce *ptr_shm_lotti = (merce(*)) ptr_shm_dettaglilotti; 
    /* per ogni merce, aggiungi in offerta o domanda un random r lotti */

    /*
    for (j = 0; j < par_SO_MERCI; j++){
		printf("Merce %d: val = %d, exp = %d\n", j, (ptr_shm_lotti + j)->val, (ptr_shm_lotti+ j)->exp);
	}
    */
    for(j=0;j<par_SO_MERCI;j++){
        r = rand() % MAX_MERCI_SPAWN_RANDOM + 0;
        tmp = ptr_shm_lotti[j].val * r;
        /* se il peso generato supera il valore di toFill, cicla fino ad un valore inferiore */
        while(tmp > toFill){
            r = r-RIDUCI_RANDOM;
            tmp = ptr_shm_lotti[j].val * r;
        }
        
        /* se la riduzione del peso scende sotto lo zero, si imposta la merce a 0 */
        if(tmp < 0 || tmp == 0){
            ptr[i][j].exp;
            (*(ptr+i)+j)->val = 0;
            (*(ptr+i)+j)->exp = NO_SCADENZA;
            tmp = 0;
        } else if(r & 1) { /* se il random generato è pari, si imposta la merce in domanda */
            (*(ptr+i)+j)->val = -r;
            (*(ptr+i)+j)->exp = NO_SCADENZA;
        } else {         /* altrimenti si imposta in offerta */
            (*(ptr+i)+j)->val = r;
            (*(ptr+i)+j)->exp = ptr_shm_lotti[j].exp;
        }

        toFill-=tmp;
        /* se sono state generate domande e offerte pari a toFill, tutte le altre merci si impostano a 0 */
        if(toFill == 0){
            j++;
            for(;j<par_SO_MERCI;j++){
                (*(ptr+i)+j)->val = 0;
                (*(ptr+i)+j)->exp = NO_SCADENZA;
            }
            
        }
    
    }

    /* se è stato assegnato un valore per ogni merce, viene incrementato di 1 la domanda o l'offerta per ogni merce
    fino a raggiungere toFill */
    while(toFill>0){
        /* printf("%d\n",__LINE__); */
        for(j=0;j<par_SO_MERCI;j++){
            
            /* aumenta offerta */
            if((*(ptr+i)+j)->val > 0){
                if(toFill >= ptr_shm_lotti[j].val){
                    (*(ptr+i)+j)->val++;
                    toFill -= ptr_shm_lotti[j].val;
                }
            }

            /* aumenta domanda */
            if((*(ptr+i)+j)->val < 0){
                if(toFill >= ptr_shm_lotti[j].val ){
                    (*(ptr+i)+j)->val--;
                    toFill -= ptr_shm_lotti[j].val;
                }
            }
        }
    }
}

/* MERCATO: */
merce caricamerci(richiesta rkst, int spaziolibero, int scadenza, int par_SO_MERCI, void *ptr_mercato, merce* ptr_lotti){
    merce (*ptr)[par_SO_MERCI] = ptr_mercato;
    merce ritorno;
    while(rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val > spaziolibero){
        rkst.mtext.nlotti--;
    }

    if((*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->exp<scadenza){
        ritorno.val = -1;
        ritorno.exp = -1;
        return ritorno;
    }

    if((*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->val > rkst.mtext.nlotti){
        ritorno.val -= rkst.mtext.nlotti;
        ritorno.exp = (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->val;

    }else if((*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->val == rkst.mtext.nlotti){
        ritorno.val = rkst.mtext.nlotti;
        ritorno.exp = (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->exp;
        (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->val = 0;
        (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->exp = NO_SCADENZA;

    }else{
        ritorno.val = (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->val;
        ritorno.exp = (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->exp;
        (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->val = 0;
        (*(ptr+rkst.mtype)+rkst.mtext.indicemerce)->exp = NO_SCADENZA;
    }

    return ritorno;
}

/* ritorna 1 se è andata bene, 0 altrimenti */
int scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, int par_SO_MERCI, void *ptr_mercato){
    merce (*ptr)[par_SO_MERCI] = ptr_mercato;
    if(scarico.exp >= data){
        (*(ptr+indiceporto)+indicemerce)->val += scarico.val;
        (*(ptr+indiceporto)+indicemerce)->exp = NO_SCADENZA;
        /*
            aggiorna consegnata al porto
        */
        return 1;
    } else {
        /*
            aggiorna scaduta in mare;
        */
       return 0;
    }
}