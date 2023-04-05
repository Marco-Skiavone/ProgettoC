#include "definitions.h" /* contiene le altre #include */
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "master_lib.h"


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

int *child_pids;
char **argv_figli;

int PARAMETRO[QNT_PARAMETRI];

void alloca_risorse();
void distruggi_risorse();

void signal_handler(int signo);

int main(int argc, char* argv[]){
    int i, j, k;
    int n_righe_file, file_config_char;
    
    /* puntatore ad un array degli indirizzi degli id delle risorse IPC. */
    int *id_shm__queue_ptr[] = {&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste};
    /* PUNTATORE A PUNTATORI: vi verranno inseriti inizialmente i puntatori delle mem. condivise. */
    void **shm_ptrs = malloc(sizeof(void *) * 4);

    int continua_simulazione;
    int child_pid, status;
    FILE *file_config;
    richiesta r;
    argv_figli = malloc((QNT_PARAMETRI + 3)*sizeof(char*));
    /*char *argv_figli[QNT_PARAMETRI + 3];*/
    setbuf(stdout, NULL); /* unbufferizza stdout */
    clearLog();
    srand(time(NULL));
    if(freopen("out.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
    if(argc !=2){
        perror("argc != 2");
        exit(EXIT_FAILURE);
    }

    n_righe_file = atoi(argv[1]);
    
    if(n_righe_file < 1){
        perror("n_righe_file < 1");
        exit(EXIT_FAILURE);
    }

    file_config = fopen("config.txt", "r");
    TEST_ERROR

    for(i=0;i<n_righe_file;){
        if(file_config_char = fgetc(file_config) == '\n'){
            i++;
        }else if(file_config_char ==EOF){
            perror("ricerca parametri");
            fclose(file_config);
            exit(EXIT_FAILURE);
        }
    }
    
    for(i=0;i<QNT_PARAMETRI;i++){
        if(fscanf(file_config, "%d", &PARAMETRO[i]) != 1){
            perror("lettura parametro");
            fclose(file_config);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file_config);

    STAMPA_PARAMETRI

    child_pids = (int *) malloc((SO_NAVI*SO_PORTI) * sizeof(int));
    
    /* eseguo una malloc per ricevere i puntatori delle memorie condivise */
    // shm_ptrs = malloc(sizeof(void*) * 4);
    alloca_risorse();
    inizializza_dump(vptr_shm_dump, PARAMETRO);
    CAST_DUMP(vptr_shm_dump)->data = 0;
    generate_positions(SO_LATO, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti), SO_PORTI);
    
    /*
    for(i=0;i<SO_PORTI;i++){
        printf("Porto %d - x: %f y: %f\n", i, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[i].x, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[i].y);
    }
    */
    
    setUpLotto(CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti),PARAMETRO);
    
    for(i=0;i<SO_MERCI;i++){
        printf("Merce %d val %d exp %d\n", i,
            CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[i].val,
            CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[i].exp);
    }
    


    /* ---------------------------------------- */
    /* semaforo numero 1 su 2 che fa 1-0 per far scrivere le navi in m.e.*/
    sem_set_val(id_semaforo_dump,1,1);
    sem_set_val(id_semaforo_dump,0,1); 
    /* ---------------------------------------- */

    sem_set_val(id_semaforo_gestione,0,SO_PORTI+SO_NAVI);
    sem_set_val(id_semaforo_gestione,1,1);
    //printf("Set id_semaforo_gestione a %d + %d = %d\n", SO_PORTI, SO_NAVI, sem_get_val(id_semaforo_gestione, 0));

    argv_figli[0] = (char *)malloc(MAX_STR_LEN);
	argv_figli[1] = (char *)malloc(MAX_STR_LEN);

	for (i = 0; i < QNT_PARAMETRI; i++){
		argv_figli[i+2] = (char *)malloc(MAX_STR_LEN);
		sprintf(argv_figli[i+2], "%d", PARAMETRO[i]);
	}
    argv_figli[QNT_PARAMETRI + 2] = NULL;
    for(i=0;i<SO_PORTI;i++){
        
        switch(child_pids[i] = fork()){
            case -1:
                perror("fork porto");
                break;
            case 0:
                argv_figli[0] = "./porto";
				sprintf(argv_figli[1], "%d", i);
				execve("./porto", argv_figli, NULL);
                perror("execve porto");
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }

    for(int i=0;i<SO_NAVI;i++){
        
        switch( child_pids[SO_PORTI + i] =  fork()){
            case -1:
                perror("fork nave");
                break;
            case 0:
                argv_figli[0] = "./nave";
				sprintf(argv_figli[1], "%d", (i));
				execve("./nave", argv_figli, NULL);
                perror("execve nave");
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }


    
    sem_wait_zero(id_semaforo_gestione, 0);
    stampa_dump(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
    struct sigaction sa_alrm;
    sa_alrm.sa_handler = signal_handler;
    sa_alrm.sa_flags = 0;
    sigemptyset(&(sa_alrm.sa_mask));
    sigaction(SIGALRM, &sa_alrm, NULL);
    
    continua_simulazione = 1;
    do{
        alarm(1);
        if(errno && errno != EINTR)
            printf("\nErrno = %d dopo alarm: %s\n", errno, strerror(errno));
        if(!(continua_simulazione = controlla_mercato(vptr_shm_mercato, vptr_shm_dump, PARAMETRO))){
            printf("\nMASTER: Termino la simulazione per mancanza di offerte e/o di richieste!\n");
        }
        pause();
    } while(((int)(CAST_DUMP(vptr_shm_dump)->data) < SO_DAYS) && continua_simulazione);
    
    if(freopen("out.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
    for(i = 0; i < SO_NAVI+SO_PORTI; i++){
        printf("MASTER: ammazzo il figlio %d\n", child_pids[i]);
        kill(child_pids[i], SIGUSR2);
    }
    
    i=0;
    while((child_pid = wait(&status)) != -1){
        printf("Terminato figlio %d status %d\n", child_pid, WEXITSTATUS(status));
    }
    printf("Master sto uscendo con gestione = %d\n", sem_get_val(id_semaforo_gestione,0));
    printf("\n__________________________ \n\n");
    
    /* svuotiamo la coda richieste */
    i = 0;
    do {    
        r = accetta_richiesta(i, id_coda_richieste);
        if(r.mtext.indicemerce != -1){
            //printf("Porto %ld merce %d nlotti %d\n", r.mtype, r.mtext.indicemerce, r.mtext.nlotti);
        }else if(i < SO_PORTI)
            i++;
    } while (r.mtext.indicemerce != -1 || i < SO_PORTI);

    sgancia_risorse(vptr_shm_dettagli_lotti,vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
    printf("TUTTE LE SHARED_MEM SONO STATE SGANCIATE DAL MASTER!\n");
    printf("__________________________ \n\n");
    distruggi_risorse(id_shm_mercato, id_shm_dettagli_lotti, id_shm_posizioni_porti, id_shm_dump, id_coda_richieste);
    distruggi_semafori(id_semaforo_mercato, id_semaforo_dump, id_semaforo_banchine, id_semaforo_gestione);
    /* sono da liberare child_pids, ogni argv_figli[i] meno l'ultimo che è null,
     *  e argv_figli stesso => tot=(QNT_PARAMETRI + 2))+1; */
    free_ptr(child_pids, argv_figli, QNT_PARAMETRI+2);
    exit(EXIT_SUCCESS);
}

void alloca_risorse(){
    int i;
    printf("\n__________________________ \n\n");
    alloca_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    
    vptr_shm_mercato = aggancia_shm(id_shm_mercato);
    vptr_shm_dettagli_lotti = aggancia_shm(id_shm_dettagli_lotti);
    vptr_shm_posizioni_porti = aggancia_shm(id_shm_posizioni_porti);
    vptr_shm_dump = aggancia_shm(id_shm_dump);
    //printf("SHARED_MEM_MERCATO: %d\n", id_shm_mercato = alloca_shm(CHIAVE_SHAREDM_MERCATO, SIZE_SHAREDM_MERCATO));
    //printf("SHARED_MEM_DETTAGLI_LOTTI: %d\n", id_shm_dettagli_lotti = alloca_shm(CHIAVE_SHAREDM_DETTAGLI_LOTTI, SIZE_SHAREDM_DETTAGLI_LOTTI));
    //printf("SHARED_MEM_POSIZIONI_PORTI: %d\n", id_shm_posizioni_porti = alloca_shm(CHIAVE_SHAREDM_POSIZIONI_PORTI, SIZE_SHAREDM_POSIZIONI_PORTI));
    //printf("SHARED_MEM_DUMP: %d\n",id_shm_dump =  alloca_shm(CHIAVE_SHAREDM_DUMP, SIZE_SHAREDM_DUMP));
    //printf("CODA RICHIESTE: %d\n", id_coda_richieste = set_coda_richieste(CHIAVE_CODA));
    alloca_semafori(&id_semaforo_banchine, &id_semaforo_dump, &id_semaforo_gestione, &id_semaforo_mercato, PARAMETRO);
    printf("__________________________ \n\n");
}

void signal_handler(int signo){
    int i;
    switch(signo){
        case SIGALRM:
            if(CAST_DUMP(vptr_shm_dump)->data < SO_DAYS-1){
                fprintf(stderr, "\x1b[%dF\x1b[0J", 1);
                CAST_DUMP(vptr_shm_dump)->data++;
                printf("\nMASTER: Passato giorno %d su %d.\n", CAST_DUMP(vptr_shm_dump)->data, SO_DAYS);
                stampa_dump(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
                for(i = 0; i < SO_NAVI+SO_PORTI; i++)
                    { kill(child_pids[i], SIGUSR1);}
                fprintf(stderr, "La simulazione è in corso :) attendi ancora altri %d secondi...\n", (SO_DAYS - CAST_DUMP(vptr_shm_dump)->data));
            } else {
                CAST_DUMP(vptr_shm_dump)->data++;
                stampa_terminazione(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
                fprintf(stderr, "\x1b[%dF\x1b[0J", 1);
                fprintf(stderr, "Simulazione completata ^_^\n");
            }
            break;
        default: 
            perror("MASTER: giunto segnale diverso da SIGALRM!");
            exit(254);
    }
}