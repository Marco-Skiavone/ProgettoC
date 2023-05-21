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
    merce(*ptr_shm_mercato_porto)[SO_MERCI] = CAST_MERCATO(vptr_mercato);
    
    srand(getpid());
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

void spawnMerci(void* vptr_mercato, merce* ptr_lotti, void *vptr_dump, int id_sem_dump, int id_coda_richieste, int fd_fifo_coda, int PARAMETRO[], int indice){
    int i, j, nlotti, peso, esito;
    int Fill = (SO_FILL/SO_PORTI) / CAST_DUMP(vptr_dump)->rand_porti;
    merce buffer_mercato[SO_MERCI];
    richiesta r;

    bzero(buffer_mercato, SO_MERCI*sizeof(merce));
    srand(getpid());
    for(i=0;i<SO_MERCI;i++){    
        /* PROPOSTA RANDOM: potremmo provare qualcosa di simile a:  */
        nlotti = rand()% (Fill/SO_MERCI) + 1;    /* (ptr_lotti[i].val / so_merci > 1) ? ptr_lotti[i].val / so_merci : 2 */
        esito = nlotti % 3;
        peso = nlotti * ptr_lotti[i].val;
        while(peso > Fill){
            nlotti --;
            peso = nlotti * ptr_lotti[i].val;
        }
        if((CAST_MERCATO(vptr_mercato))[indice][i].val != 0){   /*caso di generazione di merci ulteriore*/
            if(nlotti > 0 && (CAST_MERCATO(vptr_mercato))[indice][i].val > 0){
                buffer_mercato[i].val = nlotti;
                buffer_mercato[i].exp = ptr_lotti[i].exp;
                Fill -= peso;
            }else if(nlotti > 0){
                buffer_mercato[i].val = -nlotti;
                buffer_mercato[i].exp = SO_DAYS+1;
                Fill -= peso;
            } else {
                buffer_mercato[i].val = 0;
                buffer_mercato[i].exp = SO_DAYS+1;
            }
        } else {    /* caso di nuova generazione di merci */
            if(nlotti > 0 && !esito){
                buffer_mercato[i].val = nlotti;
                buffer_mercato[i].exp = ptr_lotti[i].exp;
                Fill -= peso;
            }else if(nlotti > 0 && esito == 1){
                buffer_mercato[i].val = -nlotti;
                buffer_mercato[i].exp = SO_DAYS+1;
                Fill -= peso;
            } else {
                buffer_mercato[i].val = 0;
                buffer_mercato[i].exp = SO_DAYS+1;
            }
        }
    }

    /*fprintf(stderr, "porto %d, in data %d: merce generata (I STEP), fill mancante = %d\n", indice, CAST_DUMP(vptr_dump)->data, Fill);*/
    srand(time(NULL));
    while(Fill>0){
        for(i=0;i<SO_MERCI;i++){
            nlotti = rand() % Fill + 1;
            peso = nlotti * ptr_lotti[i].val;
            if(peso <= Fill && buffer_mercato[i].val > 0){
                buffer_mercato[i].val += nlotti;
                Fill -= peso;
            }else if( peso <= Fill && buffer_mercato[i].val < 0){
                buffer_mercato[i].val -= nlotti;
                Fill -= peso;
            }
            if(i == SO_MERCI-1 && buffer_mercato[i].val == 0){
                buffer_mercato[i].val += Fill;
                Fill = 0;
            }
        }
        /*fprintf(stderr, "porto %d-%d: fill = %d\n", indice, CAST_DUMP(vptr_dump)->data, Fill);*/
    }
    /*fprintf(stderr, "porto %d, in data %d: merce generata (II STEP)\n", indice, CAST_DUMP(vptr_dump)->data);*/
    /* aggiorno effettivamente il mercato */
    for(i = 0; i < SO_MERCI;i++){
        if(buffer_mercato[i].val > 0){
            (CAST_MERCATO(vptr_mercato))[indice][i].val += buffer_mercato[i].val; 
        } else if (buffer_mercato[i].val < 0){
            (CAST_MERCATO(vptr_mercato))[indice][i].val -= buffer_mercato[i].val; 
            r.mtext.indicemerce = i;
            r.mtext.nlotti = buffer_mercato[i].val;
            r.mtype = indice;
            invia_richiesta(r, id_coda_richieste, fd_fifo_coda);
        }
    }
    /*fprintf(stderr, "porto %d, in data %d: mercato aggiornato (III STEP)\n", indice, CAST_DUMP(vptr_dump)->data);*/
    /* aggiorna dump */
    sem_reserve(id_sem_dump, 0);
    for(j=0;j<SO_MERCI;j++){
        if(buffer_mercato[j].val > 0){
            CAST_MERCE_DUMP(vptr_dump)[j].presente_in_porto += buffer_mercato[j].val;
            CAST_PORTO_DUMP(vptr_dump)[indice].mercepresente += buffer_mercato[j].val;
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