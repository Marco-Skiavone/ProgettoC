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

int indice;
int PARAMETRO[QNT_PARAMETRI];

void signal_handler(int signo);

int main(int argc, char *argv[]){
    int i, j, k;

    int id_semaforo_mercato;
    int id_semaforo_gestione;
    int id_semaforo_banchine;

    int id_shm_dettagli_lotti;
    int id_shm_dump;
    int id_shm_mercato;
    int id_shm_posizioni_porti;
    int id_coda_richieste;
    int SHM_ID[4];
    int richieste_mandate = 0;
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

    trova_tutti_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    SHM_ID[0] = id_shm_mercato;
    SHM_ID[1] = id_shm_dettagli_lotti;
    SHM_ID[2] = id_shm_posizioni_porti;
    SHM_ID[3] = id_shm_dump;
    aggancia_tutte_shm(&vptr_shm_mercato, &vptr_shm_dettagli_lotti, &vptr_shm_posizioni_porti, &vptr_shm_dump, SHM_ID, PARAMETRO);
    inizializza_semafori(&id_semaforo_mercato, &id_semaforo_gestione, &id_semaforo_banchine, &id_semaforo_dump, SO_PORTI);
    inizializza_banchine(id_semaforo_banchine, indice, vptr_shm_dump, PARAMETRO);
    
    spawnMerciPorti(vptr_shm_mercato, CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti), vptr_shm_dump, id_semaforo_dump, PARAMETRO, indice);
    richieste_mandate = manda_richieste(vptr_shm_mercato, indice, id_coda_richieste, 0, PARAMETRO);
    /* si sgancia dalle memorie condivise. */
    /* si dichiara pronto e aspetta. (wait for zero) */
    sem_reserve(id_semaforo_gestione, 0);
    sem_wait_zero(id_semaforo_gestione, 0);
    do {
        pause();
        
        richieste_mandate = manda_richieste(vptr_shm_mercato, indice, id_coda_richieste, richieste_mandate, PARAMETRO);
        printf("Porto %d: richieste mandate: %d\n", indice, richieste_mandate);
    } while(1);
    sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:
            fprintf(stderr,"*** PORTO %d: ricevuto SIGUSR1: data = %d ***\n", indice, CAST_DUMP(vptr_shm_dump)->data);
            break;
        case SIGUSR2:
            fprintf(stderr,"\nPORTO %d: ricevuto SIGUSR2.\n", indice);
            sgancia_risorse(vptr_shm_dettagli_lotti, vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
            exit(EXIT_SUCCESS);
            break;
        default: 
            perror("PORTO: giunto segnale non contemplato!");
            exit(254);
    }
}