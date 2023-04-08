#include "definitions.h" /* contiene le altre #include */
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "nave_lib.h"
#include "common_lib.h"

void* vptr_shm_dettagli_lotti;
void* vptr_shm_dump;
void* vptr_shm_mercato;
void* vptr_shm_posizioni_porti;

int indice;
int PARAMETRO[QNT_PARAMETRI];

void signal_handler(int signo);

int main(int argc, char *argv[]){
    int i, j, k;
    int id_semaforo_mercato;
    int id_semaforo_gestione;
    int id_semaforo_banchine;
    int id_semaforo_dump;

    int id_shm_dettagli_lotti;
    int id_shm_dump;
    int id_shm_mercato;
    int id_shm_posizioni_porti;
    int id_coda_richieste;

    int SHM_ID[4];

    sigset_t mask1;
    struct sigaction sa;
    sa.sa_flags = 0/*SA_RESTART*/;
    sa.sa_handler = signal_handler;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask1, NULL);

    if(argc !=(2 + QNT_PARAMETRI)){
        perror("argc != 2");
        exit(EXIT_FAILURE);
    }
    indice = atoi(argv[1]);

	for (i = 2; i < argc; i++){
		PARAMETRO[i - 2] = atoi(argv[i]);
	}
    if(freopen("log_navi.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}

    trova_tutti_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    SHM_ID[0] = id_shm_mercato;
    SHM_ID[1] = id_shm_dettagli_lotti;
    SHM_ID[2] = id_shm_posizioni_porti;
    SHM_ID[3] = id_shm_dump;
    aggancia_tutte_shm(&vptr_shm_mercato, &vptr_shm_dettagli_lotti, &vptr_shm_posizioni_porti, &vptr_shm_dump, SHM_ID, PARAMETRO);
    inizializza_semafori(&id_semaforo_mercato, &id_semaforo_gestione, &id_semaforo_banchine, &id_semaforo_dump, SO_PORTI);

    sem_reserve(id_semaforo_gestione, 0);
    sem_wait_zero(id_semaforo_gestione, 0);
    
    int SEM_ID[] = {id_semaforo_banchine, id_semaforo_dump, id_semaforo_gestione, id_semaforo_mercato};
    void* VPTR_ARR[] = {vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti};

    codice_simulazione(indice, PARAMETRO, SEM_ID, id_coda_richieste, VPTR_ARR);

    sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
    exit(EXIT_SUCCESS);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:
            printf("*** NAVE %d: ricevuto SIGUSR1: data = %d ***\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            #ifdef DUMP_ME
            sem_wait_zero(id_semaforo_gestione, 1);
            #endif
            break;
        case SIGUSR2:
            printf("NAVE %d: ricevuto SIGUSR2. data: %d\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
            exit(EXIT_SUCCESS);
            break;
        default: 
            perror("NAVE: giunto segnale non contemplato!");
            exit(254);
    }
}