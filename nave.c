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

int id_semaforo_gestione; /* può essere chiamato da runME (con definizione di DUMP_ME) */

int indice;
int statoNave;
int fd_fifo;
int PARAMETRO[QNT_PARAMETRI];

void signal_handler(int signo);

int main(int argc, char *argv[]){
    int i, j, k;
    int id_semaforo_mercato, id_semaforo_banchine, id_semaforo_dump;

    int id_shm_dettagli_lotti, id_shm_dump, id_shm_mercato, id_shm_posizioni_porti;
    int id_coda_richieste, id_coda_meteo;
    int SHM_ID[4];
    int SEM_ID[4];
    void* VPTR_ARR[4];
    sigset_t mask1;
    struct sigaction sa;
    sa.sa_flags = 0;
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

    fprintf(stderr, "%s %d %d\n", __FILE__, __LINE__, getpid());
    if((fd_fifo = open(NOME_FIFO, O_WRONLY)) == -1)
        fprintf(stdout, "Nave: Errore nell'apertura della FIFO\n");
    trova_tutti_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    SHM_ID[0] = id_shm_mercato;
    SHM_ID[1] = id_shm_dettagli_lotti;
    SHM_ID[2] = id_shm_posizioni_porti;
    SHM_ID[3] = id_shm_dump;
    aggancia_tutte_shm(&vptr_shm_mercato, &vptr_shm_dettagli_lotti, &vptr_shm_posizioni_porti, &vptr_shm_dump, SHM_ID, PARAMETRO);
    id_coda_meteo = get_coda_id(CHIAVE_CODA_METEO);
    inizializza_semafori(&id_semaforo_mercato, &id_semaforo_gestione, &id_semaforo_banchine, &id_semaforo_dump, SO_PORTI);
    statoNave = NAVE_IN_MARE;
    fprintf(stderr, "%s, valore semaforo = %d\n", __FILE__, sem_get_val(id_semaforo_gestione, 0));
    sem_reserve(id_semaforo_gestione, 0);
    fprintf(stderr, "%s %d %d\n", __FILE__, __LINE__, getpid());
    sem_wait_zero(id_semaforo_gestione, 0);
    
    SEM_ID[0] = id_semaforo_banchine;
    SEM_ID[1] = id_semaforo_dump;
    SEM_ID[2] = id_semaforo_gestione;
    SEM_ID[3] = id_semaforo_mercato;
    VPTR_ARR[0] = vptr_shm_dettagli_lotti;
    VPTR_ARR[1] = vptr_shm_dump;
    VPTR_ARR[2] = vptr_shm_mercato;
    VPTR_ARR[3] = vptr_shm_posizioni_porti;
    
    fprintf(stderr, "%s %d %d\n", __FILE__, __LINE__, getpid());
    codice_simulazione(indice, PARAMETRO, SEM_ID, id_coda_richieste, VPTR_ARR, fd_fifo, id_coda_meteo, &statoNave);

    sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
    exit(EXIT_SUCCESS);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:       /* dump e terminazione */
            if(CAST_DUMP(vptr_shm_dump)->data < SO_DAYS)
                printf("*** NAVE %d: ricevuto SIGUSR1: data = %d ***\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            else 
                printf("*** ERRORE! *** NAVE %d: ricevuto SIGUSR1 in data %d\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            break;
        case SIGUSR2:       /* nanosleep da mare o porto */
            if(statoNave == NAVE_IN_PORTO){
                attesa(SO_SWELL_DURATION, 1);
            } else {
                attesa(SO_STORM_DURATION, 1);
            }
            break;
        case SIGTERM:
            printf("NAVE %d: ricevuto SIGTERM. data: %d\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            /* segnala carico perso */
            close(fd_fifo);
            sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
            break;
        default: 
            printf("NAVE: giunto segnale non contemplato!");
    }
}