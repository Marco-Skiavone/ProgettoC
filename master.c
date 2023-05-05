#include "definitions.h" /* contiene le altre #include */
#include "queue_lib.h"
#include "sem_lib.h"
#include "shm_lib.h"
#include "master_lib.h"
#include "common_lib.h"

void* vptr_shm_mercato;
void* vptr_shm_posizioni_porti;
void* vptr_shm_dettagli_lotti;
void* vptr_shm_dump;

int id_semaforo_gestione;
int id_semaforo_banchine;
int id_semaforo_dump;

int *child_pids;
int fd_fifo;
int demone_pid;
int PARAMETRO[QNT_PARAMETRI];

void signal_handler(int signo);

int main(int argc, char* argv[]){
    int i, j, k;
    int n_righe_file, file_config_char;

    int id_shm_mercato, id_shm_posizioni_porti;
    int id_shm_dettagli_lotti, id_shm_dump, id_coda_richieste;
    int SHM_ID[4];
    int id_semaforo_mercato;

    char **argv_figli, **argv_demone;
    int continua_simulazione, child_pid, status;
    FILE *file_config;
    richiesta r;
    struct sigaction sa_alrm;
    argv_figli = malloc((QNT_PARAMETRI + 3)*sizeof(char*));

    #ifdef DUMP_ME
    printf("Programma in avvio con parametro DUMP_ME abilitato!\n\n");
    #endif
    
    setbuf(stdout, NULL); /* unbufferizza stdout */
    clearLog();
    srand(time(NULL));
    if(freopen("out.txt", "a", stdout)==NULL)
        {fprintf(stderr, "freopen ha ritornato NULL");}
    if(argc !=2){
        fprintf(stderr, "argc != 2");
        exit(EXIT_FAILURE);
    }

    n_righe_file = atoi(argv[1]);
    
    if(n_righe_file < 1){
        fprintf(stderr, "n_righe_file < 1");
        exit(EXIT_FAILURE);
    }

    file_config = fopen("config.txt", "r");
    TEST_ERROR
    
    for(i=0;i<n_righe_file;){
        if(file_config_char = fgetc(file_config) == '\n'){
            i++;
        }else if(file_config_char ==EOF){
            fprintf(stderr, "ricerca parametri");
            fclose(file_config);
            exit(EXIT_FAILURE);
        }
    }
    
    for(i=0;i<QNT_PARAMETRI;i++){
        if(fscanf(file_config, "%d", &PARAMETRO[i]) != 1){
            fprintf(stderr, "ERRORE: lettura parametro");
            fclose(file_config);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file_config);
    
    STAMPA_PARAMETRI

    switch(controllo_parametri(PARAMETRO)){
        case 1: 
            break;
        case 0: 
            fprintf(stderr, "\nErrore di inserimento dei parametri: violazione dei vincoli di progetto!\n");
            exit(EXIT_FAILURE);
            break;
        case -1: 
            fprintf(stderr, "\nErrore di inserimento parametri: valori ambigui!\n");
            exit(EXIT_FAILURE);
            break;
        case -2: 
            fprintf(stderr, "\nErrore di inserimento parametri: valori ambigui & violazione dei vincoli di progetto!\n");
            exit(EXIT_FAILURE);
            break;
        default:
            fprintf(stderr, "\n**FATAL ERROR**\t(inserimento parametri: caso default di ritorno)\n");
            exit(EXIT_FAILURE);
            break;
    }

    child_pids = (int *) malloc((SO_NAVI*SO_PORTI) * sizeof(int));
    
    /* alloca_risorse */
    alloca_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, PARAMETRO);
    SHM_ID[0] = id_shm_mercato;
    SHM_ID[1] = id_shm_dettagli_lotti;
    SHM_ID[2] = id_shm_posizioni_porti;
    SHM_ID[3] = id_shm_dump;
    aggancia_tutte_shm(&vptr_shm_mercato, &vptr_shm_dettagli_lotti, &vptr_shm_posizioni_porti, &vptr_shm_dump, SHM_ID, PARAMETRO);
    alloca_semafori(&id_semaforo_banchine, &id_semaforo_dump, &id_semaforo_gestione, &id_semaforo_mercato, PARAMETRO);

    inizializza_dump(vptr_shm_dump, PARAMETRO);
    generate_positions(SO_LATO, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti), SO_PORTI);
    
    setUpLotto(CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti),PARAMETRO);
    
    for(i=0;i<SO_MERCI;i++){
        printf("Merce %d val %d exp %d\n", i,
            CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[i].val,
            CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti)[i].exp);
    }

    if((mkfifo(NOME_FIFO, 0666) == -1))
        fprintf(stderr, "Master: Errore nella creazione della FIFO!\n");
    /* ---------------------------------------- */
    /* semaforo numero 1 su 2 che fa 1-0 per far scrivere le navi in m.e.*/
    sem_set_val(id_semaforo_dump,1,1);
    sem_set_val(id_semaforo_dump,0,1); 
    /* ---------------------------------------- */

    sem_set_val(id_semaforo_gestione,0,SO_PORTI+SO_NAVI+1);

    #ifdef DUMP_ME/* definito nel caso di dump in mutua esclusione. */
    sem_set_val(id_semaforo_gestione,1,0);  
    #endif
    /* settaggio di argv_figli e fork dei processi porto e nave */
    argv_demone[0] = (char*)malloc(MAX_STR_LEN);
    argv_demone[0] = "./demone";

    switch(demone_pid = fork()){
        case -1:
            fprintf(stderr, " Linea %d: errore nella fork del demone.\n", __LINE__);
            break;
        case 0:
            execve("./demone", argv_demone, NULL);
            fprintf(stderr, "Linea %d: execve demone ha fallito!\n", __LINE__);
            break;
        default:
            break;
    }

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
    for(i=0;i<SO_NAVI;i++){
        
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

    /* Fine settaggio argv_figli e creazione dei processi.
     * Inizio attesa di sincronizzazione e partenza del loop di simulazione. */
    sem_wait_zero(id_semaforo_gestione, 0);
    stampa_dump(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
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
    
    unlink(NOME_FIFO);
    kill(demone_pid, SIGUSR2);
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
            /*printf("Porto %ld merce %d nlotti %d\n", r.mtype, r.mtext.indicemerce, r.mtext.nlotti);*/
        }else if(i < SO_PORTI)
            i++;
    } while (r.mtext.indicemerce != -1 || i < SO_PORTI);

    sgancia_risorse(vptr_shm_dettagli_lotti,vptr_shm_dump, vptr_shm_mercato, vptr_shm_posizioni_porti);
    printf("TUTTE LE SHARED_MEM SONO STATE SGANCIATE DAL MASTER!\n");
    printf("__________________________ \n\n");
    distruggi_risorse(id_shm_mercato, id_shm_dettagli_lotti, id_shm_posizioni_porti, id_shm_dump, id_coda_richieste);
    distruggi_semafori(id_semaforo_mercato, id_semaforo_dump, id_semaforo_banchine, id_semaforo_gestione);
    close(fd_fifo);
    /* sono da liberare child_pids, ogni argv_figli[i] meno l'ultimo che è null,
     *  e argv_figli stesso => tot=(QNT_PARAMETRI + 2))+1; */
    free_ptr(child_pids, argv_figli, QNT_PARAMETRI+2);
    exit(EXIT_SUCCESS);
}

void signal_handler(int signo){
    int i;
    switch(signo){
        case SIGALRM:
            #ifdef DUMP_ME
            sem_release(id_semaforo_gestione, 1);
            #endif
            controllo_scadenze_porti(CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti), vptr_shm_mercato, vptr_shm_dump, id_semaforo_dump, PARAMETRO);
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
            #ifdef DUMP_ME
            sem_reserve(id_semaforo_gestione, 1);
            #endif
        default: 
            perror("MASTER: giunto segnale diverso da SIGALRM!");
            close(fd_fifo);
            exit(254);
    }
}