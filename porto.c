#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "porto_lib.h"
#include "common_lib.h"

void* vptr_shm_dettagli_lotti;
void* vptr_shm_dump;
void* vptr_shm_mercato;
void* vptr_shm_posizioni_porti;

int id_semaforo_dump;
int id_semaforo_banchine;

int id_coda_richieste;
int indice;
int fd_fifo;
int PARAMETRO[QNT_PARAMETRI];

void signal_handler(int signo);

int main(int argc, char *argv[]){
    int i, j, k;

    int id_semaforo_mercato;
    int id_semaforo_gestione;

    int id_shm_dettagli_lotti;
    int id_shm_dump;
    int id_shm_mercato;
    int id_shm_posizioni_porti;
    int SHM_ID[4];
    struct sigaction sa;
    sigset_t mask1;
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
    TEST_ERROR
	for (i = 2; i < argc; i++){
		PARAMETRO[i - 2] = atoi(argv[i]);
	}
    TEST_ERROR

    dup2(open("log_porti.txt", O_APPEND | O_CREAT | O_WRONLY, 0666), STDERR_FILENO);
    if((fd_fifo = open(NOME_FIFO, O_WRONLY)) == -1)
        fprintf(stderr, "Porto: Errore nell'apertura della FIFO!\n");
    trova_tutti_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    SHM_ID[0] = id_shm_mercato;
    SHM_ID[1] = id_shm_dettagli_lotti;
    SHM_ID[2] = id_shm_posizioni_porti;
    SHM_ID[3] = id_shm_dump;
    aggancia_tutte_shm(&vptr_shm_mercato, &vptr_shm_dettagli_lotti, &vptr_shm_posizioni_porti, &vptr_shm_dump, SHM_ID, PARAMETRO);
    inizializza_semafori(&id_semaforo_mercato, &id_semaforo_gestione, &id_semaforo_banchine, &id_semaforo_dump, SO_PORTI);
    inizializza_banchine(id_semaforo_banchine, indice, vptr_shm_dump, PARAMETRO);
    /*
    spawnMerciPorti(vptr_shm_mercato, CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti), vptr_shm_dump, id_semaforo_dump, PARAMETRO, indice);
    manda_richieste(vptr_shm_mercato, indice, id_coda_richieste, PARAMETRO, fd_fifo);
    */
    /* si sgancia dalle memorie condivise. */
    /* si dichiara pronto e aspetta. (wait for zero) */
    sem_reserve(id_semaforo_gestione, 0);
    fprintf(stderr, "%s %d %d\n", __FILE__, __LINE__, getpid());

    sem_wait_zero(id_semaforo_gestione, 0);
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
    do {
        pause();
        
    } while(1);
}

void signal_handler(int signo){
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
    fprintf(stderr, "%s %d %d \n", __FILE__, __LINE__, signo);

    switch(signo){
        case SIGUSR1:   /* DUMP e fine simulazione */
            if(CAST_DUMP(vptr_shm_dump)->data < SO_DAYS)
                fprintf(stderr,"*** PORTO %d: ricevuto SIGUSR1: data = %d ***\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            else {
                fprintf(stderr,"\nPORTO %d: ricevuto SIGUSR1 terminazione.\n", indice);
                close(fd_fifo);
                sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
                exit(EXIT_SUCCESS);
            }
            break;
        case SIGUSR2:   /* spawn merci del porto */
            fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
            spawnMerciRand(vptr_shm_mercato, vptr_shm_dettagli_lotti, vptr_shm_dump, id_semaforo_dump, PARAMETRO, indice, id_coda_richieste, fd_fifo);
            break;
        case SIGINT:    /* swell -> mareggiata */
            {
                struct timespec time_to_wait, rem;
                int value;
                time_to_wait.tv_sec = (int)(SO_SWELL_DURATION / 24);
                time_to_wait.tv_nsec = (SO_SWELL_DURATION % 24) * ((int)(1000000000/24));
                value = sem_get_val(id_semaforo_banchine, indice);
                sem_set_val(id_semaforo_banchine, indice, 0);   /* porta a zero il semaforo */
                /* NANOSLEEP DI TOT SECONDI */ 
                if(nanosleep(&time_to_wait, &rem) != 0){
                    while(rem.tv_sec != 0 && rem.tv_nsec != 0)
                        nanosleep(&rem, &rem);
                }
                sem_set_val(id_semaforo_banchine, indice, value); /* riporta il semaforo al valore precedente */
            }
            break;
        default:/*
            perror("PORTO: giunto segnale non contemplato!");
            exit(254);
        */ 
            break;
    }
}