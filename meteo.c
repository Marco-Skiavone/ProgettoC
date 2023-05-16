#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"

void signal_handler(int signo);
void mareggiata_porto();
void tempesta_nave();

int PARAMETRO[QNT_PARAMETRI];
int id_semaforo_dump;
int id_semaforo_banchine;
int *porti_pids;
posizione_navi *vettore_pids_navi;

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
    if((fd_fifo_pids = open(FIFO_PIDS, O_RDONLY, 0666))==NULL){
        perror("Errore fifo pids");
    }
    porti_pids = (int *) malloc(SO_PORTI * sizeof(int));
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
    //AGGANCIA SEMAFORI NECESSARI
    //SEMAFORO PER DIRE CHE È PRONTO
    //SEMAFORO PER ATTESA PARTENZA SIMULAZIONE

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
            tempesta_nave();
            mareggiata_porto();
            break;
        case SIGUSR2:
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}

void tempesta_nave(){
    int i;
    while(1){
        i = rand() % SO_NAVI + 0;
        if(vettore_pids_navi[i].indice_porto == -1){
            kill(vettore_pids_navi[i].pid, SIGUSR2);
            break;
        }
    }
}

void mareggiata_porto(){
    int i, porto_mareggiato;
    porto_mareggiato = rand() % SO_PORTI + 0;
    kill(porti_pids[porto_mareggiato], SIGINT);
    for(i=0;i<SO_NAVI;i++){
        if(vettore_pids_navi[i].indice_porto == porto_mareggiato){
            kill(vettore_pids_navi[i].pid, SIGUSR2);
        }
    }

}