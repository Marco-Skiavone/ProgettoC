#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"

void signal_handler(int signo);

int PARAMETRO[QNT_PARAMETRI];
int id_semaforo_gestione;
int *porti_pids;
posizione_navi *vettore_pids_navi;
int *porti_mareggiati;
int *navi_tempesta;
int data;

int main(int argc, char *argv[]){
    int id_coda_b;
    int i;
    sigset_t mask1;
    struct sigaction sa;
    messaggio_posizioni mes;
    int fd_fifo_pids;
    srand(getpid());
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask1, NULL);

    if(argc != QNT_PARAMETRI){
        perror(("argc meteo"));
    }
    for(i=1;i<argc;i++){
        PARAMETRO[i-1] = atoi(argv[i]);
    }
    
    if((fd_fifo_pids = open(FIFO_PIDS, O_RDONLY, 0666)) == -1){
        perror("Errore fifo pids");
    }

    if(freeopen("log_dump.txt", "a", stdout) == NULL){
        perror("freeopen log_dump stampa meteo");
    }

    porti_mareggiati = (int*) malloc(SO_DAYS * sizeof(int));
    navi_tempesta = (int) malloc(SO_DAYS * sizeof(int));
    porti_pids = (int *) malloc(SO_PORTI * sizeof(int));
    data = 0;
    vettore_pids_navi = (posizione_navi*) malloc(SO_NAVI * sizeof(posizione_navi));
    for(i=0;i<SO_PORTI;i++){
        read(fd_fifo_pids, porti_pids[i], sizeof(int));
    }
    for(i=0;i<SO_NAVI;i++){
        read(fd_fifo_pids, vettore_pids_navi[i].pid, sizeof(int));
    }
    close(fd_fifo_pids);
    for(i=0;i<SO_NAVI;i++){ vettore_pids_navi[i].indice_porto=-1; }

    id_coda_b = get_coda_id(CHIAVE_CODA_METEO);
    /* Aggancia il semaforo */
    id_semaforo_gestione = sem_find(CHIAVE_SEM_GESTIONE, 2);
    sem_reserve(id_semaforo_gestione, 0);
    sem_wait_zero(id_semaforo_gestione, 0);

    do{
        msgrcv(id_coda_b, &mes, sizeof(messaggio_posizioni), 0, 0);
        for(i=0;i<SO_NAVI;i++){
            if(vettore_pids_navi[i].pid == mes.posizione.pid){
                vettore_pids_navi[i].indice_porto = mes.posizione.indice_porto;
            }
        }
    }while(1);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:
            navi_tempesta[data] = tempesta_nave();
            porti_mareggiati[data] = mareggiata_porto();
            sem_reserve(id_semaforo_gestione, 1);
            /* STAMPA DATI METEO */
            data++;
            break;
        case SIGUSR2:
            stampa_meteo_fine_simulazione(navi_tempesta, porti_mareggiati, data);
            /* free delle malloc */
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}