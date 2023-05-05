#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "porto_lib.h"

void inizializza_banchine(int sem_id, int indice, void * vptr_shm_dump, int PARAMETRO[]){
    int nbanchine;
	srand(getpid());
	nbanchine = rand() % SO_BANCHINE + 1;
    CAST_PORTO_DUMP(vptr_shm_dump)[indice].banchineoccupate = 0;
    CAST_PORTO_DUMP(vptr_shm_dump)[indice].banchinetotali = nbanchine;
    sem_set_val(sem_id, indice, nbanchine);
}

void spawnMerciPorti(void* vptr_mercato, merce* ptr_lotti, void *vptr_dump, int id_sem_dump, int PARAMETRO[], int indice){
    int i, j, nlotti, peso;
    int Fill = SO_FILL/SO_PORTI;
    srand(getpid());
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_mercato);
    for(i=0;i<SO_MERCI;i++){
        nlotti = rand()% 10 + 1;
        peso = nlotti * ptr_lotti[i].val;
        while(peso > Fill){
            nlotti --;
            peso = nlotti * ptr_lotti[i].val;
        }
        if(nlotti > 0 && nlotti%2==0){
            ptr_shm_mercato_porto[indice][i].val = nlotti;
            ptr_shm_mercato_porto[indice][i].exp = ptr_lotti[i].exp;
            Fill -= peso;
        }else if(nlotti > 0 && nlotti%2==1){
            ptr_shm_mercato_porto[indice][i].val = -nlotti;
            ptr_shm_mercato_porto[indice][i].exp = SO_DAYS+1;
            Fill -= peso;
        }else{
            ptr_shm_mercato_porto[indice][i].val = 0;
            ptr_shm_mercato_porto[indice][i].exp = SO_DAYS+1;
        }
    }
    while(Fill>0){
        for(i=0;i<SO_MERCI;i++){
            nlotti = rand() % 10 + 1;
            peso = nlotti * ptr_lotti[i].val;
            if(peso <= Fill && ptr_shm_mercato_porto[indice][i].val > 0){
                ptr_shm_mercato_porto[indice][i].val += nlotti;
                Fill -= peso;
            }else if( peso <= Fill && ptr_shm_mercato_porto[indice][i].val < 0){
                ptr_shm_mercato_porto[indice][i].val -= nlotti;
                Fill -= peso;
            }
        }
    }

    sem_reserve(id_sem_dump, 0);
    for(j=0;j<SO_MERCI;j++){
        if(ptr_shm_mercato_porto[indice][j].val > 0){
            CAST_MERCE_DUMP(vptr_dump)[j].presente_in_porto += ptr_shm_mercato_porto[indice][j].val;
            CAST_PORTO_DUMP(vptr_dump)[indice].mercepresente += ptr_shm_mercato_porto[indice][j].val;
        }  
    }
    sem_release(id_sem_dump, 0);
}

void manda_richieste(void* vptr_shm_mercato, int indice, int coda_id, int PARAMETRO[]){
    int i;
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_shm_mercato);
    richiesta r;
    r.mtype = indice;
    for(i=0;i<SO_MERCI;i++){
        if(ptr_shm_mercato_porto[indice][i].val < 0){
            r.mtext.indicemerce = i;
            r.mtext.nlotti = - ptr_shm_mercato_porto[indice][i].val;
            invia_richiesta(r,coda_id);
        }
    }
}