#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "queue_lib.h"

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

// int DATA;
int indice;
int PARAMETRO[QNT_PARAMETRI];
void inizializza_risorse();
void signal_handler(int signo);
void sgancia_risorse();
void spawnMerciPorti(int nmerci, void* vptr_shm_mercato, merce* ptr_dettagli_lotti, int Fill, int indice);
void manda_richieste(int nmerci, void* vptr_shm_mercato, int indice, int coda_id);
void inizializza_banchine(int sem_id, int indice, int so_banchine);
int main(int argc, char *argv[]){

    int i, j, k;
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);


    if(argc !=(2 + QNT_PARAMETRI)){
        perror("argc != 2");
        exit(EXIT_FAILURE);
    }
    indice = atoi(argv[1]);
    TEST_ERROR
	for (i = 2; i < argc; i++){
		PARAMETRO[i - 2] = atoi(argv[i]);

	}
    TEST_ERROR

    inizializza_risorse();


    //printf("Porto %d - x: %f y: %f\n", indice, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice].x, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[indice].y);

    spawnMerciPorti(SO_MERCI, vptr_shm_mercato, CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti),(SO_FILL/SO_PORTI), indice);
    manda_richieste(SO_MERCI, vptr_shm_mercato, indice, id_coda_richieste);

    inizializza_banchine(id_semaforo_banchine, indice, SO_BANCHINE);
    //printf("Porto %d nbanchine %d\n", indice, sem_get_val(id_semaforo_banchine, indice));

    
    sgancia_risorse();

    sem_reserve(id_semaforo_gestione, 0);
    sem_wait_zero(id_semaforo_gestione, 0);
    // printf("Porto %d sto uscendo con gestione = %d\n", indice, sem_get_val(id_semaforo_gestione, 0));
    do {
        pause();
    } while(1);
    exit(EXIT_SUCCESS);
}

void manda_richieste(int nmerci, void* vptr_shm_mercato, int indice, int coda_id){

    int i;
    merce(*ptr_shm_mercato_porto)[nmerci] = CAST_MERCATO(vptr_shm_mercato);
    richiesta r;
    r.mtype = indice;
    for(i=0;i<nmerci;i++){
        if(ptr_shm_mercato_porto[indice][i].val < 0){
            r.mtext.indicemerce = i;
            r.mtext.nlotti = - ptr_shm_mercato_porto[indice][i].val;
            invia_richiesta(r,coda_id);
        }
    }
}


void spawnMerciPorti(int nmerci, void* vptr_shm_mercato, merce* ptr_dettagli_lotti, int Fill, int indice){
    srand(getpid());
    int i, j, nlotti, peso;
    merce(*ptr_shm_mercato_porto)[nmerci] = CAST_MERCATO(vptr_shm_mercato);
    for(i=0;i<nmerci;i++){
        nlotti = rand()% 10 + 1;
        peso = nlotti * ptr_dettagli_lotti[i].val;
        while(peso > Fill){
            nlotti --;
            peso = nlotti * ptr_dettagli_lotti[i].val;
        }
        if(nlotti > 0 && nlotti%2==0){
            ptr_shm_mercato_porto[indice][i].val = nlotti;
            ptr_shm_mercato_porto[indice][i].exp = ptr_dettagli_lotti[i].exp;
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
        for(i=0;i<nmerci;i++){
            nlotti = rand() % 10 + 1;
            peso = nlotti * ptr_dettagli_lotti[i].val;
            if(peso <= Fill && ptr_shm_mercato_porto[indice][i].val > 0){
                ptr_shm_mercato_porto[indice][i].val += nlotti;
                Fill -= peso;
            }else if( peso <= Fill && ptr_shm_mercato_porto[indice][i].val < 0){
                ptr_shm_mercato_porto[indice][i].val -= nlotti;
                Fill -= peso;
            }
        }
    }
    /*
    */
    sem_reserve(id_semaforo_gestione, 1);
    // sleep(indice);
    printf("Porto %d\n", indice);
    for(j=0;j<SO_MERCI;j++){
        printf("Merce %d nlotti %d scadenza %d\n", j, ptr_shm_mercato_porto[indice][j].val, ptr_shm_mercato_porto[indice][j].exp);
    }
    sem_release(id_semaforo_gestione, 1);
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
    id_semaforo_mercato = sem_find(CHIAVE_SEM_MERCATO,SO_PORTI);
    id_semaforo_gestione = sem_find(CHIAVE_SEM_GESTIONE, 2);
    id_semaforo_banchine = sem_find(CHIAVE_SEM_BANCHINE, SO_PORTI);
    id_semaforo_dump = sem_find(CHIAVE_SEM_DUMP,2);
    id_coda_richieste = get_coda_id(CHIAVE_CODA);
}

void sgancia_risorse(){
    sgancia_shm(vptr_shm_mercato);
    sgancia_shm(vptr_shm_dettagli_lotti);
    sgancia_shm(vptr_shm_posizioni_porti);
    sgancia_shm(vptr_shm_dump);
}

void inizializza_banchine(int sem_id, int indice, int so_banchine){
    srand(getpid());
    int nbanchine;
	nbanchine = rand() % SO_BANCHINE + 1;
    sem_set_val(sem_id, indice, nbanchine);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:
            printf("*** PORTO %d: ricevuto SIGUSR1: data = %d ***\n", indice, /*DATA*/CAST_DUMP(vptr_shm_dump)->data);
            break;
        case SIGUSR2:
            printf("\nPORTO %d: ricevuto SIGUSR2.\n", indice);
            exit(EXIT_SUCCESS);
            break;
        default: 
            perror("PORTO: giunto segnale non contemplato!");
            exit(254);
    }
}