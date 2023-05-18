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



void spawnMerciRand(void* vptr_mercato, merce* ptr_lotti, void *vptr_dump, int id_sem_dump, int PARAMETRO[], int indice, int coda_id, int fd_fifo){
    int porti_generanti;
    int merci_da_generare;
    int fill, i, j, nlotti, peso;
    int mask_vector[SO_MERCI];
    merce stato_mercato[SO_MERCI];
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_mercato);
    srand(getpid());
    richiesta r;
    porti_generanti = CAST_DUMP(vptr_dump)->porti_generanti;
    
    fill = (SO_FILL/SO_DAYS)/porti_generanti;
    merci_da_generare = rand()% SO_MERCI + 0;
    for(i=0;i<SO_MERCI-1;i++){ mask_vector[i] = 0;}
    mask_vector[SO_MERCI-1] = 1;
    for(i=0;i<merci_da_generare;){
        if(mask_vector[i] == 0){ mask_vector[i] = 1; i++; }
    }
    for(i=0;i<SO_MERCI;i++){
        stato_mercato[i] = ptr_shm_mercato_porto[indice][i];
    }
    for(i=0;i<SO_MERCI;i++){    
        if(mask_vector[i] == 1){

            /* PROPOSTA RANDOM: potremmo provare qualcosa di simile a:  */
            nlotti = rand()% 10 + 1;    /* (ptr_lotti[i].val / so_merci > 1) ? ptr_lotti[i].val / so_merci : 2 */
            peso = nlotti * ptr_lotti[i].val;
            while(peso > fill){
                nlotti --;
                peso = nlotti * ptr_lotti[i].val;
          }
            if(nlotti > 0 && nlotti%2==0){
                ptr_shm_mercato_porto[indice][i].val = nlotti;
                ptr_shm_mercato_porto[indice][i].exp = ptr_lotti[i].exp;
                fill -= peso;
            }else if(nlotti > 0 && nlotti%2==1){
                ptr_shm_mercato_porto[indice][i].val = -nlotti;
                ptr_shm_mercato_porto[indice][i].exp = SO_DAYS+1;
                fill -= peso;
            }else{
                ptr_shm_mercato_porto[indice][i].val = 0;
                ptr_shm_mercato_porto[indice][i].exp = SO_DAYS+1;
            }
        }

    }
    while(fill>0){
        for(i=0;i<SO_MERCI;i++){
            if(mask_vector[i]==1){
                nlotti = rand() % 10 + 1;
                peso = nlotti * ptr_lotti[i].val;
                if(peso <= fill && ptr_shm_mercato_porto[indice][i].val > 0){
                    ptr_shm_mercato_porto[indice][i].val += nlotti;
                    fill -= peso;
                }else if( peso <= fill && ptr_shm_mercato_porto[indice][i].val < 0){
                    ptr_shm_mercato_porto[indice][i].val -= nlotti;
                    fill -= peso;
                }
            }
        }
    }

    r.mtype = indice;
    sem_reserve(id_sem_dump, 0);
    for(i=0;i<SO_MERCI;i++){
        if(stato_mercato[i].val < 0 && stato_mercato[i].val > ptr_shm_mercato_porto[indice][i].val){
            r.mtext.indicemerce = i;
            r.mtext.nlotti = ptr_shm_mercato_porto[indice][i].val - stato_mercato[i].val;
            invia_richiesta(r, coda_id, fd_fifo);
        }else if(stato_mercato[i].val > 0 && stato_mercato[i].val < ptr_shm_mercato_porto[indice][i].val){
            CAST_MERCE_DUMP(vptr_dump)[i].presente_in_porto += (ptr_shm_mercato_porto[indice][i].val - stato_mercato[i].val);
            CAST_PORTO_DUMP(vptr_dump)[indice].mercepresente += (ptr_shm_mercato_porto[indice][i].val - stato_mercato[i].val);
        }
    }
    sem_release(id_sem_dump, 0);

}

void spawnMerciPorti(void* vptr_mercato, merce* ptr_lotti, void *vptr_dump, int id_sem_dump, int PARAMETRO[], int indice){
    int i, j, nlotti, peso;
    int Fill = SO_FILL/SO_PORTI;
    srand(getpid());
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_mercato);
    for(i=0;i<SO_MERCI;i++){    
        /* PROPOSTA RANDOM: potremmo provare qualcosa di simile a:  */
        nlotti = rand()% 10 + 1;    /* (ptr_lotti[i].val / so_merci > 1) ? ptr_lotti[i].val / so_merci : 2 */
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