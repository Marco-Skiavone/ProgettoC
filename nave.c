#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "nave_lib.h"
#include "common_lib.h"

void* vptr_shm_dettagli_lotti;
void* vptr_shm_dump;
void* vptr_shm_mercato;
void* vptr_shm_posizioni_porti;

int id_semaforo_gestione; /* può essere chiamato da runME (con definizione di DUMP_ME) */

int indice;
int fd_fifo;
int PARAMETRO[QNT_PARAMETRI];

void signal_handler(int signo);

int main(int argc, char *argv[]){
    int i, j, k;
    int id_semaforo_mercato;
    int id_semaforo_banchine;
    int id_semaforo_dump;

    int id_shm_dettagli_lotti;
    int id_shm_dump;
    int id_shm_mercato;
    int id_shm_posizioni_porti;
    int id_coda_richieste;
    int SHM_ID[4];
    int SEM_ID[4];
    void* VPTR_ARR[4];
    sigset_t mask1;
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
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

    if((fd_fifo = open(NOME_FIFO, O_WRONLY)) == -1)
        fprintf(stdout, "Nave: Errore nell'apertura della FIFO\n");
    trova_tutti_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    SHM_ID[0] = id_shm_mercato;
    SHM_ID[1] = id_shm_dettagli_lotti;
    SHM_ID[2] = id_shm_posizioni_porti;
    SHM_ID[3] = id_shm_dump;
    aggancia_tutte_shm(&vptr_shm_mercato, &vptr_shm_dettagli_lotti, &vptr_shm_posizioni_porti, &vptr_shm_dump, SHM_ID, PARAMETRO);
    inizializza_semafori(&id_semaforo_mercato, &id_semaforo_gestione, &id_semaforo_banchine, &id_semaforo_dump, SO_PORTI);

    sem_reserve(id_semaforo_gestione, 0);
    sem_wait_zero(id_semaforo_gestione, 0);
    
    SEM_ID[0] = id_semaforo_banchine;
    SEM_ID[1] = id_semaforo_dump;
    SEM_ID[2] = id_semaforo_gestione;
    SEM_ID[3] = id_semaforo_mercato;
    VPTR_ARR[0] = vptr_shm_dettagli_lotti;
    VPTR_ARR[1] = vptr_shm_dump;
    VPTR_ARR[2] = vptr_shm_mercato;
    VPTR_ARR[3] = vptr_shm_posizioni_porti;

    codice_simulazione(indice, PARAMETRO, SEM_ID, id_coda_richieste, VPTR_ARR, fd_fifo);

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
        case SIGTERM:
            printf("NAVE %d: ricevuto SIGTERM. data: %d\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            close(fd_fifo);
            sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
            exit(EXIT_SUCCESS);
            break;
        default: 
            perror("NAVE: giunto segnale non contemplato!");
            exit(EXIT_FAILURE);
            break;
    }
}