#include "definitions.h" /* contiene le altre #include */
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "nave_lib.h"

void* vptr_shm_mercato;
int id_shm_mercato;

void* vptr_shm_posizioni_porti;
int id_shm_posizioni_porti;

void* vptr_shm_dettagli_lotti;
int id_shm_dettagli_lotti;

void* vptr_shm_dump;
int id_shm_dump;

int id_semaforo_mercato;
int id_semaforo_gestione;
int id_semaforo_banchine;
int id_semaforo_dump;

int id_coda_richieste;

int indice;
int PARAMETRO[QNT_PARAMETRI];


/* elementi di debug per la nave */
int DEB_porti_attraccati;
int DEB_porti_lasciati;
int DEB_porto_ultima_destinazione;
/*-------------------------------*/

void inizializza_risorse();
void signal_handler(int signo);

point generate_random_point(int lato);
double calcola_distanza(point p1, point p2);
int calcola_porto_piu_vicino(point p, point* ptr_shm_posizioni_porti, int so_porti, int so_lato);
void naveinmare();
void naveinporto();

void attesa(double val, int divisore);

void codice_simulazione();

/* aggiorna il dump sulle merci e sul porto che hanno subito modifiche !!!! */
void aggiorna_dump_carico(int indiceporto, merce_nave* carico, int caricato, int spazio_libero);

void scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, int so_merci, void* vptr_shm_mercato, void* vptr_shm_dump_porto);

int main(int argc, char *argv[]){
    int i, j, k;
    struct sigaction sa;
    sa.sa_flags = 0/*SA_RESTART*/;
    sa.sa_handler = signal_handler;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigset_t mask1;
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask1, NULL);

    // /*DATA*/CAST_DUMP(vptr_shm_dump)->data = 0;
    if(argc !=(2 + QNT_PARAMETRI)){
        perror("argc != 2");
        exit(EXIT_FAILURE);
    }
    indice = atoi(argv[1]);

	for (i = 2; i < argc; i++){
		PARAMETRO[i - 2] = atoi(argv[i]);

	}
    inizializza_risorse();
    sem_reserve(id_semaforo_gestione, 0);
    sem_wait_zero(id_semaforo_gestione, 0);
    
    if(freopen("log_navi.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
    codice_simulazione();

    sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
    exit(EXIT_SUCCESS);
}

void aggiorna_dump_carico(int indiceporto, merce_nave* carico, int caricati, int spazio_libero){
    int i;
    //printf("Entro aggiorna dump sem reserve \n");
    sem_reserve(id_semaforo_dump, 0);

    if(spazio_libero != SO_CAPACITY){ 
        for(i = 0; i < caricati; i++){
            CAST_PORTO_DUMP(vptr_shm_dump)[indiceporto].mercespedita += carico[i].mer.val;
            CAST_PORTO_DUMP(vptr_shm_dump)[indiceporto].mercepresente -= carico[i].mer.val;
        
            CAST_MERCE_DUMP(vptr_shm_dump)[carico[i].indice].presente_in_nave += carico[i].mer.val;
            CAST_MERCE_DUMP(vptr_shm_dump)[carico[i].indice].presente_in_porto -= carico[i].mer.val;
        }
    }
    sem_release(id_semaforo_dump, 0);
    //printf("Esco aggiorna dump sem reserve \n");
}

void scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, int so_merci, void* vptr_shm_mercato_porto, void* vptr_shm_dump_porto){    
    /* aggiorno mercato shm se possibile */
    if(scarico.exp >= data && CAST_MERCATO(vptr_shm_mercato_porto)[indiceporto][indicemerce].val <= -scarico.val){
        CAST_MERCATO(vptr_shm_mercato_porto)[indiceporto][indicemerce].val += scarico.val;
    } else {
        perror("scarica_merci()");
    }
    sem_reserve(id_semaforo_dump, 0);
    if(scarico.exp >= data){ 
        CAST_MERCE_DUMP(vptr_shm_dump)[indicemerce].consegnata += scarico.val /* * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[indicemerce].val*/;
        CAST_MERCE_DUMP(vptr_shm_dump)[indicemerce].presente_in_nave -= scarico.val /* * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[indicemerce].val*/;
        CAST_PORTO_DUMP(vptr_shm_dump)[indiceporto].mercericevuta += scarico.val /* * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[indicemerce].val*/;
    }else{
        CAST_MERCE_DUMP(vptr_shm_dump)[indicemerce].scaduta_in_nave += scarico.val /* * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[indicemerce].val*/;
        CAST_MERCE_DUMP(vptr_shm_dump)[indicemerce].presente_in_nave -= scarico.val /* * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[indicemerce].val*/;
    }
    sem_release(id_semaforo_dump, 0);
}

point generate_random_point(int lato) {
    srand(getpid());
    int mant, p_intera;
    point p;
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


void codice_simulazione(){
    int i, j, k, indice_destinazione, indice_porto_attraccato, i_carico=0, skip=0;
    int reqlett=0, spaziolibero = SO_CAPACITY, lotti_scartati = 0, noncaricare = 0;
    int datascarico;
    double distanza, tempo_carico = 0;
    point posizione;
    richiesta r;
    merce_nave carico[MAX_CARICO];
    bzero(carico, MAX_CARICO*sizeof(merce_nave));

    DEB_porti_attraccati = DEB_porti_lasciati = 0;
    DEB_porto_ultima_destinazione = -1;

    /* Genera la posizione della nave, 
     * trova il porto più vicino e ci va. */
    posizione = generate_random_point(SO_LATO);
    indice_destinazione = calcola_porto_piu_vicino(posizione, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti), SO_PORTI, SO_LATO);
    distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice_destinazione]);
    printf("Nave %d in posizione x:%f y:%f indice porto piu vicino %d x:%f y;%f\n", indice, posizione.x, posizione.y, indice_destinazione, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice_destinazione].x, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice_destinazione].y );
    printf("Nave %d inizia il viaggio\n", indice);
    attesa(distanza, SO_SPEED);
    /* richiede la banchina e una volta dentro aggiorna il dump */
    /*sem_reserve(id_semaforo_banchine, indice_destinazione);*/
    richiedi_banchina(id_semaforo_banchine, indice_destinazione);
    DEB_porti_attraccati++;
    DEB_porto_ultima_destinazione = indice_destinazione; 
    stato_nave(DN_MV_PORTO, id_semaforo_dump, vptr_shm_dump, indice);
    indice_porto_attraccato = indice_destinazione;
    posizione = CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice_porto_attraccato];
    printf("Nave %d ha ricevuto una banchina al porto %d\n", indice, indice_destinazione);

    while(1){
        /* Il primo do-while esegue la ricerca della prima richiesta da accettare,
         * in base alle risorse del porto di attracco. */
        sem_reserve(id_semaforo_mercato, indice_porto_attraccato);

        do{
            //STAMPA_DEBUG
            r = accetta_richiesta(-1, id_coda_richieste);
            if(r.mtext.indicemerce == -1){
                perror("coda vuota");
                exit(255);
            }
            //STAMPA_DEBUG
            if(CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val > 0){
                printf("Richiesta da porto %d merce %d lotti %d al porto %d con val %d\n", (int)r.mtype,r.mtext.indicemerce, r.mtext.nlotti, indice_porto_attraccato, (CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val));
                distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[r.mtype]);
                while(r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val > spaziolibero){
                    r.mtext.nlotti--;
                    lotti_scartati++;
                }
                
                while(r.mtext.nlotti > CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val){
                    r.mtext.nlotti--;
                    lotti_scartati++;
                }
                tempo_carico += ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;

                if((CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].exp > (/*DATA*/CAST_DUMP(vptr_shm_dump)->data + tempo_carico + (distanza/SO_SPEED))) && r.mtext.nlotti > 0){
                    //STAMPA_DEBUG
                    carico[i_carico].indice = r.mtext.indicemerce;
                    carico[i_carico].mer.val = r.mtext.nlotti;
                    carico[i_carico].mer.exp = CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].exp;

                    spaziolibero -= r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val;

                    CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val -= r.mtext.nlotti;
                    //STAMPA_DEBUG
                    printf("nave %d ha caricato %d lotti di merce %d spaziolibero: %d\n", indice, carico[i_carico].mer.val, carico[i_carico].indice, spaziolibero);

                    i_carico++;

                    if(lotti_scartati > 0){
                        r.mtext.nlotti =  lotti_scartati;
                        invia_richiesta(r, id_coda_richieste);
                        lotti_scartati = 0;
                    }

                    indice_destinazione = r.mtype;
                    break;
                }else{
                    tempo_carico = 0;
                    r.mtext.nlotti += lotti_scartati;
                    invia_richiesta(r, id_coda_richieste);
                    lotti_scartati = 0;
                    reqlett++;
                }
            }else{
                invia_richiesta(r, id_coda_richieste);
                lotti_scartati = 0;
                reqlett++;
                //STAMPA_DEBUG
            }
        }while(reqlett < MAX_REQ_LETTE);

        if(reqlett == MAX_REQ_LETTE){
            skip=1;
        }
        
        /* Inizia il secondo do-while, che deve accettare le richieste
         * del porto associato alla prima accettata. */
        do{
            if(skip){
                //STAMPA_DEBUG
                printf("Nave %d deve skippare porto %d\n", indice, indice_porto_attraccato);
                indice_destinazione = rand() % (SO_PORTI-1) + 0;
                distanza = calcola_distanza(posizione, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice_destinazione]);
                skip = 0;
                break;
            }
            r = accetta_richiesta(indice_destinazione, id_coda_richieste);
            if(r.mtext.indicemerce == -1){
                break;
            }
            if(CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val > 0){

                while(r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val > spaziolibero){
                    r.mtext.nlotti--;
                    lotti_scartati++;
                }
                while(r.mtext.nlotti > CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val){
                    r.mtext.nlotti--;
                    lotti_scartati++;
                }
                tempo_carico += ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
                if((CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].exp > (tempo_carico + (distanza/SO_SPEED) + /*DATA*/CAST_DUMP(vptr_shm_dump)->data)) && r.mtext.nlotti > 0){
                    //STAMPA_DEBUG
                    noncaricare = 0;
                    for(j=0;j<i_carico;j++){
                        if(CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[carico[j].indice].exp < ((distanza/SO_SPEED) + tempo_carico + /*DATA*/CAST_DUMP(vptr_shm_dump)->data)){
                            noncaricare = 1;
                            break;
                        }
                    }
                    if(noncaricare){
                        //STAMPA_DEBUG
                        tempo_carico += ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
                        r.mtext.nlotti += lotti_scartati;
                        invia_richiesta(r, id_coda_richieste);
                        lotti_scartati = 0;
                    }else{
                        //STAMPA_DEBUG
                        spaziolibero -= r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val;
                        carico[i_carico].indice = r.mtext.indicemerce;
                        carico[i_carico].mer.val = r.mtext.nlotti;
                        carico[i_carico].mer.exp = CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].exp;
                        CAST_MERCATO(vptr_shm_mercato)[indice_porto_attraccato][r.mtext.indicemerce].val -= r.mtext.nlotti;
                        //STAMPA_DEBUG
                        printf("nave %d ha caricato %d lotti di merce %d spaziolibero: %d\n", indice, carico[i_carico].mer.val, carico[i_carico].indice, spaziolibero);
                        i_carico++;
                        if(lotti_scartati > 0){
                            r.mtext.nlotti =  lotti_scartati;
                            invia_richiesta(r, id_coda_richieste);
                            lotti_scartati = 0;
                        }
                    }
                }else{
                    tempo_carico -= ((r.mtext.nlotti * CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[r.mtext.indicemerce].val) / SO_LOADSPEED) *2;
                    r.mtext.nlotti += lotti_scartati;
                    invia_richiesta(r, id_coda_richieste);
                    lotti_scartati = 0;
                }
                reqlett++;
            }else{
                invia_richiesta(r, id_coda_richieste);
                lotti_scartati = 0;
                reqlett++;
            }
        }while(reqlett < MAX_REQ_LETTE);
        

        /* a questo punto rilascia mercato shm e carica le risorse,
         * dopodiché aggiorna il dump sul carico e salpa per il porto di destinazione */
        sem_release(id_semaforo_mercato, indice_porto_attraccato);

        printf("Nave %d inizia a caricare\n", indice);
        aggiorna_dump_carico(indice_porto_attraccato, carico, i_carico, spaziolibero);
        attesa((SO_CAPACITY - spaziolibero), SO_LOADSPEED);
        printf("Nave %d ha caricato\n", indice);
        sem_release(id_semaforo_banchine, indice_porto_attraccato);
        if(spaziolibero == SO_CAPACITY){
			stato_nave(DN_PORTO_MV, id_semaforo_dump, vptr_shm_dump, indice);
        }else{
            stato_nave(DN_PORTO_MC, id_semaforo_dump, vptr_shm_dump, indice);
        }
        printf("Nave %d parte\n", indice);
        DEB_porti_lasciati += 1;
        attesa(distanza,SO_SPEED);
        printf("Nave %d arriva\n", indice);
        posizione = CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice_destinazione];
        indice_porto_attraccato = indice_destinazione;

        //sem_reserve(id_semaforo_banchine, indice_porto_attraccato);
        richiedi_banchina(id_semaforo_banchine, indice_porto_attraccato);
        DEB_porti_attraccati += 1;
        DEB_porto_ultima_destinazione = indice_porto_attraccato;
        printf("Nave %d attraccata al porto %d\n", indice, indice_porto_attraccato);
        if(spaziolibero == SO_CAPACITY){
            stato_nave(DN_MV_PORTO, id_semaforo_dump, vptr_shm_dump, indice);
        }else{
            stato_nave(DN_MC_PORTO, id_semaforo_dump, vptr_shm_dump, indice);
        }
        attesa((SO_CAPACITY-spaziolibero), SO_LOADSPEED);
        /* salva la data di scarico della merce */
        datascarico = CAST_DUMP(vptr_shm_dump)->data;

        /* invertendo sem_release e sem_reserve del dump, ho forse creato capacità potenziale di deadlock ???? */
        /* se non l'avessi fatto, avremmo modifiche al dump in zone critiche senza mutua esclusione !!!! */
        int data1 = CAST_DUMP(vptr_shm_dump)->data;
        sem_reserve(id_semaforo_mercato,indice_porto_attraccato);

        printf("nave %d in attesa dal giorno %d: giorno attuale: %d\n", indice, data1, CAST_DUMP(vptr_shm_dump)->data);


        printf("Nave %d inizia a scaricare al porto %d. giorno %d\n", indice, indice_porto_attraccato, CAST_DUMP(vptr_shm_dump)->data);
        for(j=0;j<i_carico;j++){
            scaricamerci(carico[j].mer, indice_porto_attraccato, carico[j].indice, datascarico, SO_MERCI, vptr_shm_mercato, vptr_shm_dump);
        }
        printf("Nave %d ha scaricato al porto %d. giorno %d\n", indice, indice_porto_attraccato, CAST_DUMP(vptr_shm_dump)->data);

        sem_release(id_semaforo_mercato, indice_porto_attraccato);
        
        /* resetto i valori dei parametri necessari all'iterazione successiva */
        spaziolibero = SO_CAPACITY;
        tempo_carico = 0;
        i_carico = 0;
        skip = 0;
        reqlett = 0;
    }   
}

void inizializza_risorse(){
    id_shm_mercato = find_shm(CHIAVE_SHAREDM_MERCATO, SIZE_SHAREDM_MERCATO);
    vptr_shm_mercato = aggancia_shm(id_shm_mercato);
    id_shm_dettagli_lotti = find_shm(CHIAVE_SHAREDM_DETTAGLI_LOTTI, SIZE_SHAREDM_DETTAGLI_LOTTI);
    vptr_shm_dettagli_lotti = aggancia_shm(id_shm_dettagli_lotti);
    id_shm_posizioni_porti = find_shm(CHIAVE_SHAREDM_POSIZIONI_PORTI, SIZE_SHAREDM_POSIZIONI_PORTI);
    vptr_shm_posizioni_porti = aggancia_shm(id_shm_posizioni_porti);
    id_shm_dump = find_shm(CHIAVE_SHAREDM_DUMP, SIZE_SHAREDM_DUMP);
    vptr_shm_dump = aggancia_shm(id_shm_dump);
    inizializza_semafori(&id_semaforo_mercato, &id_semaforo_gestione, &id_semaforo_banchine, &id_semaforo_dump, SO_PORTI);
    id_coda_richieste = get_coda_id(CHIAVE_CODA);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:
            printf("*** NAVE %d: ricevuto SIGUSR1: data = %d ***\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            break;
        case SIGUSR2:
            printf("NAVE %d: ricevuto SIGUSR2. data: %d\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            printf("#DEB - Nave %d:\n-porti_attraccati: %d\n-porti_lasciati: %d\n-ultimo_indice: %d\n", indice, DEB_porti_attraccati, DEB_porti_lasciati, DEB_porto_ultima_destinazione);
            sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
            exit(EXIT_SUCCESS);
            break;
        default: 
            perror("NAVE: giunto segnale non contemplato!");
            exit(254);
    }
}
