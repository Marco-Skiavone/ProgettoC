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
    srand(getpid());
    int toFillOff, toFillReq, maxRandGen, maxRandInc;
    int i,j, nlotti, peso;
    int maskVet[SO_MERCI];
    maxRandGen = RANDOM_GEN_LOTTI;
    maxRandInc = RANDOM_INC_LOTTI;
    toFillOff = SO_FILL/SO_PORTI;
    toFillReq = SO_FILL/SO_PORTI;
    if(SO_MERCI>1){
        for(i=0;i<SO_MERCI;i++){
            if(rand()&1)maskVet[i] = 1;
            else maskVet[i] = -1;
        }
        if(maskVet[SO_MERCI-1] == maskVet[SO_MERCI-2]){
            maskVet[SO_MERCI-1] = -maskVet[SO_MERCI-1];
        }
    }else{
        if(rand()&1){
            maskVet[0] = 1;
            toFillReq = 0;
        }else{
            maskVet[0] = -1;
            toFillOff = 0;
        }
    }
    
    if(maxRandGen == 0) maxRandGen = 2;
    if(maxRandInc == 0) maxRandInc = 1;
    fprintf(stderr, "Max Random Gen: %d\n", maxRandGen);
    fprintf(stderr, "Max Random Inc: %d\n", maxRandInc);
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_mercato);
    for(i=0;i<SO_MERCI;i++){
        nlotti = (rand() % maxRandGen + 1);
        peso = nlotti * ptr_lotti[i].val;
        if(maskVet[i]==1){
            while(peso > toFillOff){
                nlotti--;
                peso = nlotti * ptr_lotti[i].val;
            }
            ptr_shm_mercato_porto[indice][i].val = nlotti;
            ptr_shm_mercato_porto[indice][i].exp = ptr_lotti[i].exp;
            toFillOff -= peso;
        }else{
            while(peso > toFillReq){
                nlotti--;
                peso = nlotti * ptr_lotti[i].val;
            }
            ptr_shm_mercato_porto[indice][i].val = -nlotti;
            ptr_shm_mercato_porto[indice][i].exp = SO_DAYS+1;
            toFillReq -= peso;
        }


    }
    while(toFillOff>0){
        for(i=0;i<SO_MERCI;i++){
            if(maskVet[i]==1){
                nlotti = rand() % maxRandInc + 1;
                peso = nlotti * ptr_lotti[i].val;
                while(peso > toFillOff){
                    nlotti--;
                    peso = nlotti * ptr_lotti[i].val;
                }
                ptr_shm_mercato_porto[indice][i].val += nlotti;
                toFillOff -= peso;
                /*fprintf(stderr, "toFillOff: %d\n", toFillOff );*/
            }   
        }
    }
    fprintf(stderr, "%s  %d %d\n", __FILE__,indice, __LINE__);
    for(i=0;i<SO_MERCI;i++){
        fprintf(stderr, "%d maskVet[%d] = %d\n", getpid(), i, maskVet[i]);
    }
    while(toFillReq>0){
        for(i=0;i<SO_MERCI;i++){
            if(maskVet[i]==-1){
                nlotti = rand() % maxRandInc + 1;
                peso = nlotti * ptr_lotti[i].val;
                while(peso > toFillReq){
                    nlotti--;
                    peso = nlotti * ptr_lotti[i].val;
                }
                ptr_shm_mercato_porto[indice][i].val -= nlotti;
                toFillReq -= peso;
            }
        }
    }
    fprintf(stderr, "%s  %d %d\n", __FILE__,indice, __LINE__);
    /*
    int i, j, nlotti, peso;
    int Fill = SO_FILL/SO_PORTI;
    srand(getpid());
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_mercato);
    for(i=0;i<SO_MERCI;i++){    
        
        nlotti = rand()% RANDOM_GEN_LOTTI + 1;
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
    */
    sem_reserve(id_sem_dump, 0);
    for(j=0;j<SO_MERCI;j++){
        if(ptr_shm_mercato_porto[indice][j].val > 0){
            CAST_MERCE_DUMP(vptr_dump)[j].presente_in_porto += ptr_shm_mercato_porto[indice][j].val;
            CAST_PORTO_DUMP(vptr_dump)[indice].mercepresente += ptr_shm_mercato_porto[indice][j].val;
        }  
    }
    sem_release(id_sem_dump, 0);
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
}

void manda_richieste(void* vptr_shm_mercato, int indice, int coda_id, int PARAMETRO[], int fd_fifo){
    int i;
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_shm_mercato);
    richiesta r;
    r.mtype = indice;
    for(i=0;i<SO_MERCI;i++){
        if(ptr_shm_mercato_porto[indice][i].val < 0){
            r.mtext.indicemerce = i;
            r.mtext.nlotti = - ptr_shm_mercato_porto[indice][i].val;
            invia_richiesta(r,coda_id, fd_fifo);
        }
    }
}