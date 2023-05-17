#include "definitions.h"
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "meteo_lib.h"

void signal_handler(int signo);

int PARAMETRO[QNT_PARAMETRI];
int id_semaforo_gestione;
int *porti_pids;
posizione_navi *vettore_pids_navi;
int *porti_mareggiati, *navi_tempesta;
void *vptr_shm_dump;
int id_coda_meteo;

int main(int argc, char *argv[]){
    int i, fd_fifo_pids;
    int id_shm_dump;
    sigset_t mask1;
    struct sigaction sa;
    messaggio_posizioni mes;
    
    srand(getpid());
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigemptyset(&mask1);
    sigaddset(&mask1, SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &mask1, NULL);


    if(argc != (QNT_PARAMETRI+1)){
        perror(("argc meteo"));
    }
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);

    for(i=1;i<argc;i++){
        PARAMETRO[i-1] = atoi(argv[i]);
    }
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);

    if((fd_fifo_pids = open(FIFO_PIDS, O_RDONLY, 0666)) == -1){
        fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
        perror("Errore fifo pids");
    }
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);

    if(freopen("log_dump.txt", "a", stdout) == NULL){
        perror("freeopen log_dump stampa meteo");
    }
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);


    vptr_shm_dump = aggancia_shm(find_shm(CHIAVE_SHAREDM_DUMP, SIZE_SHAREDM_DUMP));

    porti_mareggiati = (int*) malloc(SO_DAYS * sizeof(int));
    navi_tempesta = (int) malloc(SO_DAYS * sizeof(int));
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

    id_coda_meteo = get_coda_id(CHIAVE_CODA_METEO);
    /* Aggancia il semaforo */
    id_semaforo_gestione = sem_find(CHIAVE_SEM_GESTIONE, 2);

    sem_reserve(id_semaforo_gestione, 0);
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
    sem_wait_zero(id_semaforo_gestione, 0);
    
    do{
       pause();
    /* maelstorm */
    }while(1);
}

void signal_handler(int signo){
    switch(signo){
        case SIGUSR1:
            {
            messaggio_posizioni mes;
            while(msgrcv(id_coda_meteo, &mes, sizeof(messaggio_posizioni), 0, IPC_NOWAIT)!=-1){
                int i;
                for(i=0;i<SO_NAVI;i++){
                    if(vettore_pids_navi[i].pid == mes.posizione.pid){
                        vettore_pids_navi[i].indice_porto = mes.posizione.indice_porto;
                    }
                }
            }
            navi_tempesta[CAST_DUMP(vptr_shm_dump)->data] = tempesta_nave(vettore_pids_navi, PARAMETRO);
            porti_mareggiati[CAST_DUMP(vptr_shm_dump)->data] = mareggiata_porto(porti_pids, vettore_pids_navi, PARAMETRO);
            
            sem_reserve(id_semaforo_gestione, 1);
            stampa_meteo(navi_tempesta[CAST_DUMP(vptr_shm_dump)->data], porti_mareggiati[CAST_DUMP(vptr_shm_dump)->data]);
            /* STAMPA DATI METEO */
            break;
            }
        case SIGUSR2:
            stampa_meteo_fine_simulazione(navi_tempesta, porti_mareggiati, CAST_DUMP(vptr_shm_dump)->data);
            sgancia_shm(vptr_shm_dump);
            /* free delle malloc */
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }
}