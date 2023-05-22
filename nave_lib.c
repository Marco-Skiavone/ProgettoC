#include "nave_lib.h"
#include "queue_lib.h"


point generate_random_point_nave(int lato) {
    int mant, p_intera;
    point p;
    srand(getpid());
    p_intera = mant = rand()%lato;

	p.x = ((double)mant/lato) + (p_intera*getppid()%lato);
	p_intera = mant = rand()%lato;
	p.y = ((double)mant/lato) + ((p_intera*getppid()%lato));
    return p;
}

double calcola_distanza(point p1, point p2){
    double a, b;
    a = (p1.x - p2.x);
    b = (p1.y - p2.y);
    a = a*a;
    b = b*b;
    return sqrt(a+b);
}

int calcola_porto_piu_vicino(point p, point* ptr_shm_posizioni_porti, int so_porti, int so_lato){
    int i, indicemin;
    double distmin, distanza;
    distmin = so_lato*so_lato;
    for(i=0;i<so_lato;i++){
        distanza = calcola_distanza(p, ptr_shm_posizioni_porti[i]);
        if(distanza < distmin){
            indicemin = i;
            distmin = distanza;
        }
    }
    return indicemin;
}

void codice_simulazione(int indice, int PARAMETRO[], int SEM_ID[], int id_coda_richieste, void* VPTR_ARR[], int fd_fifo){

    int i, j, k, indice_destinazione, indice_porto_attraccato, i_carico=0;
    int reqlett=0, spaziolibero = SO_CAPACITY, lotti_scartati = 0, noncaricare = 0;
    int datascarico;
    double distanza, tempo_carico = 0;
    point posizione;
    richiesta r;
    merce_nave carico[MAX_CARICO];
    bzero(carico, MAX_CARICO*sizeof(merce_nave));
    posizione = avvia_nave(indice, PARAMETRO, SEM_ID , VPTR_ARR, &indice_porto_attraccato);
    while(1){
        /* Il primo do-while esegue la ricerca della prima richiesta da accettare,

         * in base alle risorse del porto di attracco. */
        sem_reserve(ID_SEMAFORO_MERCATO, indice_porto_attraccato);
        r = esamina_porto(indice, PARAMETRO, SEM_ID, id_coda_richieste, VPTR_ARR, &lotti_scartati, &indice_porto_attraccato, &reqlett, posizione, &spaziolibero, &tempo_carico,
        carico, &i_carico, &indice_destinazione, fd_fifo);        

        if(reqlett == MAX_REQ_LETTE){
            printf("Nave %d deve skippare porto %d\n", indice, indice_porto_attraccato);
            indice_destinazione = rand() % (SO_PORTI-1) + 0;
            distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[indice_destinazione]);
        }else{
            /* Inizia il secondo do-while, che deve accettare le richieste
            * del porto associato alla prima accettata. */
            carica_dal_porto(indice, PARAMETRO, id_coda_richieste, VPTR_ARR, r, posizione, &indice_destinazione, &indice_porto_attraccato, &lotti_scartati, &spaziolibero, &tempo_carico,
            &i_carico, &reqlett, carico, fd_fifo);
            distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[indice_destinazione]);
        }

        /* a questo punto rilascia mercato shm e carica le risorse,
         * dopodiché aggiorna il dump sul carico e salpa per il porto di destinazione */
        sem_release(ID_SEMAFORO_MERCATO, indice_porto_attraccato);
        
        aggiorna_dump_carico(VPTR_SHM_DUMP, indice_porto_attraccato, carico, i_carico, spaziolibero, ID_SEMAFORO_DUMP, PARAMETRO);
        
        attesa((SO_CAPACITY - spaziolibero), SO_LOADSPEED);
        
        sem_release(ID_SEMAFORO_BANCHINE, indice_porto_attraccato);

        if(spaziolibero == SO_CAPACITY){
			stato_nave(DN_PORTO_MV, ID_SEMAFORO_DUMP, VPTR_SHM_DUMP, indice);
        }else{
            stato_nave(DN_PORTO_MC, ID_SEMAFORO_DUMP, VPTR_SHM_DUMP, indice);
        }

        printf("Nave %d parte\n", indice);
        attesa(distanza,SO_SPEED);
        printf("Nave %d arriva\n", indice);
        
        posizione = CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[indice_destinazione];
        indice_porto_attraccato = indice_destinazione;

        attracco_e_scarico(indice, PARAMETRO, SEM_ID, VPTR_ARR, &spaziolibero, &i_carico, &tempo_carico, &reqlett, &indice_porto_attraccato, carico);
    }   
}


point avvia_nave(int indice, int PARAMETRO[], int SEM_ID[], void* VPTR_ARR[], int *indice_porto_attraccato){
    int indice_destinazione;
    point posizione;
    double distanza;
    
    /* Genera la posizione della nave, 
     * trova il porto più vicino e ci va. */
    posizione = generate_random_point_nave(SO_LATO);
    indice_destinazione = calcola_porto_piu_vicino(posizione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI), SO_PORTI, SO_LATO);
    distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[indice_destinazione]);
    printf("\nNave %d in posizione x:%f y:%f indice porto piu vicino %d x:%f y;%f\n", indice, posizione.x, posizione.y, indice_destinazione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[indice_destinazione].x, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[indice_destinazione].y );
    printf("Nave %d inizia il viaggio\n", indice);
    attesa(distanza, SO_SPEED);
    /* richiede la banchina e una volta dentro aggiorna il dump */
    richiedi_banchina(ID_SEMAFORO_BANCHINE, indice_destinazione);
    stato_nave(DN_MV_PORTO, ID_SEMAFORO_DUMP, VPTR_SHM_DUMP, indice);
    *indice_porto_attraccato = indice_destinazione;
    posizione = CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[*indice_porto_attraccato];
    printf("Nave %d ha ricevuto una banchina al porto %d\n", indice, indice_destinazione);
    return posizione;
}

richiesta esamina_porto(int indice, int PARAMETRO[], int SEM_ID[], int id_coda_richieste, void* VPTR_ARR[], int* lotti_scartati, int *indice_porto_attraccato, int *reqlett, point posizione, int *spaziolibero, double *tempo_carico, merce_nave carico[], int *i_carico, int *indice_destinazione, int fd_fifo){
    richiesta r;
    double distanza;
    do{
        r = accetta_richiesta(-1, id_coda_richieste);
        if(r.mtext.indicemerce == -1){
            kill(getpid(), SIGTERM);
        }
        if(CAST_MERCATO(VPTR_SHM_MERCATO)[*indice_porto_attraccato][r.mtext.indicemerce].val > 0){
            distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[r.mtype]);
            while(r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val > *spaziolibero){
                r.mtext.nlotti--;
                (*lotti_scartati)++;
            }
            
            while(r.mtext.nlotti > CAST_MERCATO(VPTR_SHM_MERCATO)[*indice_porto_attraccato][r.mtext.indicemerce].val){
                r.mtext.nlotti--;
                (*lotti_scartati)++;
            }
            *tempo_carico += ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
            if((CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].exp > (CAST_DUMP(VPTR_SHM_DUMP)->data + *tempo_carico + (distanza/SO_SPEED))) && r.mtext.nlotti > 0){
                /* STAMPA_DEBUG */
                carico[*i_carico].indice = r.mtext.indicemerce;
                carico[*i_carico].mer.val = r.mtext.nlotti;
                carico[*i_carico].mer.exp = CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].exp;
                *spaziolibero -= r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val;
                CAST_MERCATO(VPTR_SHM_MERCATO)[*indice_porto_attraccato][r.mtext.indicemerce].val -= r.mtext.nlotti;
                printf("nave %d sta caricando %d lotti di merce %d spaziolibero: %d\n", indice, carico[*i_carico].mer.val, carico[*i_carico].indice, *spaziolibero);
                (*i_carico)++;
                if(*lotti_scartati > 0){
                    r.mtext.nlotti =  *lotti_scartati;
                    invia_richiesta(r, id_coda_richieste, fd_fifo);
                    *lotti_scartati = 0;
                }
                *indice_destinazione = r.mtype;
                break;
            }else{
                *tempo_carico = 0;
                r.mtext.nlotti += *lotti_scartati;
                invia_richiesta(r, id_coda_richieste, fd_fifo);
                *lotti_scartati = 0;
                (*reqlett)++;
            }
        }else{
            invia_richiesta(r, id_coda_richieste, fd_fifo);
            *lotti_scartati = 0;
            (*reqlett)++;
        }
    }while((*reqlett) < MAX_REQ_LETTE);
    return r;
}

void carica_dal_porto(int indice, int PARAMETRO[], int id_coda_richieste, void* VPTR_ARR[], richiesta r, point posizione, int *indice_destinazione, int *indice_porto_attraccato, int *lotti_scartati, int *spaziolibero, double *tempo_carico, int *i_carico, int *reqlett, merce_nave carico[], int fd_fifo){
    double distanza;
    int noncaricare = 0, j;
    distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(VPTR_SHM_POSIZIONI_PORTI)[r.mtype]);
    do{
        r = accetta_richiesta(*indice_destinazione, id_coda_richieste);
        if(r.mtext.indicemerce == -1){
            break;
        }
        if(CAST_MERCATO(VPTR_SHM_MERCATO)[*indice_porto_attraccato][r.mtext.indicemerce].val > 0){
            while(r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val > *spaziolibero){
                r.mtext.nlotti--;
                (*lotti_scartati)++;
            }
            while(r.mtext.nlotti > CAST_MERCATO(VPTR_SHM_MERCATO)[*indice_porto_attraccato][r.mtext.indicemerce].val){
            r.mtext.nlotti--;
                (*lotti_scartati)++;
            }
            *tempo_carico += ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
            if((CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].exp > (*tempo_carico + (distanza/SO_SPEED) + CAST_DUMP(VPTR_SHM_DUMP)->data)) && r.mtext.nlotti > 0){
                /* STAMPA_DEBUG */
                noncaricare = 0;
                for(j=0;j<*i_carico;j++){
                    if(CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[carico[j].indice].exp < ((distanza/SO_SPEED) + *tempo_carico + CAST_DUMP(VPTR_SHM_DUMP)->data)){
                        noncaricare = 1;
                        break;
                    }
                }
                if(noncaricare){
                    /* STAMPA_DEBUG */
                    *tempo_carico += ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
                    r.mtext.nlotti += *lotti_scartati;
                    invia_richiesta(r, id_coda_richieste, fd_fifo);
                    *lotti_scartati = 0;
                }else{
                    *spaziolibero -= r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val;
                    carico[*i_carico].indice = r.mtext.indicemerce;
                    carico[*i_carico].mer.val = r.mtext.nlotti;
                    carico[*i_carico].mer.exp = CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].exp;
                    CAST_MERCATO(VPTR_SHM_MERCATO)[*indice_porto_attraccato][r.mtext.indicemerce].val -= r.mtext.nlotti;
                    printf("nave %d sta caricando %d lotti di merce %d spaziolibero: %d\n", indice, carico[*i_carico].mer.val, carico[*i_carico].indice, *spaziolibero);
                    (*i_carico)++;
                    if(*lotti_scartati > 0){
                        r.mtext.nlotti =  *lotti_scartati;
                        invia_richiesta(r, id_coda_richieste, fd_fifo);
                        *lotti_scartati = 0;
                    }
                }
            }else{
                *tempo_carico -= ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(VPTR_SHM_DETTAGLI_LOTTI)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
                r.mtext.nlotti += *lotti_scartati;
                invia_richiesta(r, id_coda_richieste, fd_fifo);
                *lotti_scartati = 0;
            }
            (*reqlett)++;
        }else{
            invia_richiesta(r, id_coda_richieste, fd_fifo);
            *lotti_scartati = 0;
            (*reqlett)++;
        }
    }while(*reqlett < MAX_REQ_LETTE);    
}

void attracco_e_scarico(int indice, int PARAMETRO[], int SEM_ID[],void* VPTR_ARR[], int *spaziolibero, int *i_carico, double *tempo_carico, int *reqlett, int *indice_porto_attraccato, merce_nave carico[]){
    int j, datascarico, data1;
    richiedi_banchina(ID_SEMAFORO_BANCHINE, *indice_porto_attraccato);
    printf("Nave %d attraccata al porto %d\n", indice, *indice_porto_attraccato);
    if(*spaziolibero == SO_CAPACITY){
        stato_nave(DN_MV_PORTO, ID_SEMAFORO_DUMP, VPTR_SHM_DUMP, indice);
    }else{
        stato_nave(DN_MC_PORTO, ID_SEMAFORO_DUMP, VPTR_SHM_DUMP, indice);
    }
    attesa((SO_CAPACITY-*spaziolibero), SO_LOADSPEED);
    /* salva la data di scarico della merce */
    datascarico = CAST_DUMP(VPTR_SHM_DUMP)->data;
    /* invertendo sem_release e sem_reserve del dump, ho forse creato capacità potenziale di deadlock ???? */
    /* se non l'avessi fatto, avremmo modifiche al dump in zone critiche senza mutua esclusione !!!! */
    data1 = CAST_DUMP(VPTR_SHM_DUMP)->data;
    sem_reserve(ID_SEMAFORO_MERCATO,*indice_porto_attraccato);
    printf("nave %d in attesa dal giorno %d: giorno attuale: %d\n", indice, data1, CAST_DUMP(VPTR_SHM_DUMP)->data);
    if(*spaziolibero != SO_CAPACITY)
        printf("Nave %d inizia a scaricare al porto %d. giorno %d\n", indice, *indice_porto_attraccato, CAST_DUMP(VPTR_SHM_DUMP)->data);
    for(j=0;j<*i_carico;j++){
        scaricamerci(carico[j].mer, *indice_porto_attraccato, carico[j].indice, datascarico, VPTR_SHM_MERCATO, VPTR_SHM_DUMP, ID_SEMAFORO_DUMP, PARAMETRO);
    }
    if(*spaziolibero != SO_CAPACITY)
        printf("Nave %d ha scaricato al porto %d. giorno %d\n", indice, *indice_porto_attraccato, CAST_DUMP(VPTR_SHM_DUMP)->data);
     sem_release(ID_SEMAFORO_MERCATO, *indice_porto_attraccato);
    
    /* resetto i valori dei parametri necessari all'iterazione successiva */
    *spaziolibero = SO_CAPACITY;
    *tempo_carico = 0;
    *i_carico = 0;
    *reqlett = 0;
}

void richiedi_banchina(int id_semaforo_banchine, int indice_porto){
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    sem_reserve(id_semaforo_banchine, indice_porto);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void stato_nave(int stato, int id_semaforo_dump, void *vptr_shm_dump, int indice){
	switch(stato){
        case DN_MV_PORTO:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviscariche--;
            ((dump*)vptr_shm_dump)->nd.naviporto++;
            sem_release(id_semaforo_dump,1);
            break;
        case DN_MC_PORTO:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.navicariche--;
            ((dump*)vptr_shm_dump)->nd.naviporto++;
            sem_release(id_semaforo_dump,1);
            break;
        case DN_PORTO_MV:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviporto--;
            ((dump*)vptr_shm_dump)->nd.naviscariche++;
            sem_release(id_semaforo_dump,1);
            break;
        case DN_PORTO_MC: 
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviporto--;
            ((dump*)vptr_shm_dump)->nd.navicariche++;
            sem_release(id_semaforo_dump,1);
            break;
        default:
            perror("**** ERRORE! Caso default di stato_nave()");
            break;
    }
}

void attesa(double val, int divisore) {
    struct timespec tempo;
    sigset_t mask, oldmask;
    int ret;
    double attesa_nanosleep;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    attesa_nanosleep = (val / divisore);	
    tempo.tv_sec = (__time_t)((attesa_nanosleep));
	tempo.tv_nsec = (__time_t)((attesa_nanosleep - ((__time_t)attesa_nanosleep))*1000000000); 

    ret = nanosleep(&tempo, NULL);
    if (ret != 0) {
        if (errno == EINTR){
            perror("nanosleep interrotta da un segnale");
        }else{
            perror("nanosleep fallita");
        }
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void aggiorna_dump_carico(void *vptr_dump, int indiceporto, merce_nave* carico, int caricati, int spazio_libero,  int id_sem_dump, int PARAMETRO[]){
    int i;
    sem_reserve(id_sem_dump, 0);
    if(spazio_libero != SO_CAPACITY){ 
        for(i = 0; i < caricati; i++){
            CAST_PORTO_DUMP(vptr_dump)[indiceporto].mercespedita += carico[i].mer.val;
            CAST_PORTO_DUMP(vptr_dump)[indiceporto].mercepresente -= carico[i].mer.val;
        
            CAST_MERCE_DUMP(vptr_dump)[carico[i].indice].presente_in_nave += carico[i].mer.val;
            CAST_MERCE_DUMP(vptr_dump)[carico[i].indice].presente_in_porto -= carico[i].mer.val;
        }
    }
    sem_release(id_sem_dump, 0);
}

void scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, void* vptr_mercato, void* vptr_dump, int id_sem_dump, int PARAMETRO[]){    
    /* aggiorno mercato shm se possibile */
    if(scarico.exp >= data && CAST_MERCATO(vptr_mercato)[indiceporto][indicemerce].val <= -scarico.val){
        CAST_MERCATO(vptr_mercato)[indiceporto][indicemerce].val += scarico.val;
    } else if(CAST_MERCATO(vptr_mercato)[indiceporto][indicemerce].val > -scarico.val) {
        fprintf(stderr, "Errore in scarica_merci()\n");
    }
    sem_reserve(id_sem_dump, 0);
    if(scarico.exp >= data){ 
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].consegnata += scarico.val;
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].presente_in_nave -= scarico.val;
        CAST_PORTO_DUMP(vptr_dump)[indiceporto].mercericevuta += scarico.val;
    }else{
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].scaduta_in_nave += scarico.val;
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].presente_in_nave -= scarico.val;
    }
    sem_release(id_sem_dump, 0);
}