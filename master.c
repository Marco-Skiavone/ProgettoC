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
char **argv_figli, **argv_demone, **argv_meteo;
int fd_fifo_demone, fd_fifo_pids;
int demone_pid;
int PARAMETRO[QNT_PARAMETRI];
int continua_simulazione;

void signal_handler(int signo);

int main(int argc, char* argv[]){
    int i, j, k;
    int n_righe_file, file_config_char;

    int id_shm_mercato, id_shm_posizioni_porti;
    int id_shm_dettagli_lotti, id_shm_dump, id_coda_richieste, id_coda_meteo;
    int SHM_ID[4];
    int id_semaforo_mercato;
    int *mask_porti_generanti;
    int child_pid, meteo_pid, status;
    FILE *file_config;
    richiesta r;
    struct sigaction sa;
    argv_figli = (char**)malloc(SIZE_ARGV_FIGLI*sizeof(char*));
    argv_demone = (char**)malloc(sizeof(char*)*SIZE_ARGV_DEMONE);
    argv_meteo = (char**)malloc(SIZE_ARGV_METEO*sizeof(char*));
    
    setbuf(stdout, NULL); /* unbufferizza stdout */
    clearLog();
    srand(time(NULL));
    if(freopen("out.txt", "a", stdout)==NULL)
        {fprintf(stdout, "freopen ha ritornato NULL");}
    printf("%s %d\n", __FILE__, __LINE__);
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

    
    child_pids = (int *) malloc((SO_NAVI*SO_PORTI + 1) * sizeof(int));
    mask_porti_generanti = (int *) malloc(SO_PORTI * sizeof(int));

    /* alloca_risorse */
    alloca_id(&id_shm_mercato, &id_shm_dettagli_lotti, &id_shm_posizioni_porti, &id_shm_dump, &id_coda_richieste, &id_coda_meteo, PARAMETRO);
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
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
    if(mkfifo(NOME_FIFO, 0666) == -1)
        fprintf(stderr, "Master: Errore nella creazione della FIFO del demone!\n");
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
    if(mkfifo(FIFO_PIDS, 0666) == -1)
        fprintf(stderr, "Master: Errore nella creazione della FIFO del meteo!\n");
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);

    if((fd_fifo_pids = open(FIFO_PIDS, O_RDWR)) == -1){
        fprintf(stderr, "Master: Errore nella open della FIFO del meteo!\n");
    }else{
        fprintf(stderr, "%s %d\n", __FILE__, __LINE__);    
    }
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);
    /* ---------------------------------------- */
    /* semaforo numero 1 su 2 che fa 1-0 per far scrivere le navi in m.e.*/
    sem_set_val(id_semaforo_dump,1,1);
    sem_set_val(id_semaforo_dump,0,1); 
    /* ---------------------------------------- */

    sem_set_val(id_semaforo_gestione,0,SO_PORTI+SO_NAVI+2); /* navi, porti, demone e meteo */
    sem_set_val(id_semaforo_gestione,1,0);  

    /* settaggio di argv per demone, meteo e "figli" (navi e porti). */
    argv_demone[0] = (char*)malloc(MAX_STR_LEN);
    argv_demone[0] = strcpy(argv_demone[0], "./demone");
    argv_demone[1] = (char*)malloc(3*sizeof(char));
    sprintf(argv_demone[1], "%d", CHIAVE_SEM_GESTIONE);
    argv_demone[2] = NULL;
    
    argv_meteo[0] = (char*)malloc(MAX_STR_LEN);
    argv_meteo[0] = strcpy(argv_meteo[0], "./meteo");
    argv_meteo[1] = (char*)malloc(MAX_STR_LEN);
    argv_meteo[QNT_PARAMETRI+1] = NULL;

    argv_figli[0] = (char *)malloc(MAX_STR_LEN);
	argv_figli[1] = (char *)malloc(MAX_STR_LEN);
	for (i = 0; i < QNT_PARAMETRI; i++){
		argv_figli[i+2] = (char *)malloc(MAX_STR_LEN);
		sprintf(argv_figli[i+2], "%d", PARAMETRO[i]);
        argv_meteo[i+1] = (char *)malloc(MAX_STR_LEN);
		sprintf(argv_meteo[i+1], "%d", PARAMETRO[i]);
	}
    argv_figli[QNT_PARAMETRI + 2] = NULL;
    /* fork dei processi demone, meteo, porto e nave */
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

    switch(meteo_pid = fork()){
        case -1:
            fprintf(stderr, " Linea %d: errore nella fork del meteo.\n", __LINE__);
            break;
        case 0:
            execve("./meteo", argv_meteo, NULL);
            fprintf(stderr, "Linea %d: execve meteo ha fallito!\n", __LINE__);
            break;
        default:
            break;
    }

    for(i=0;i<SO_PORTI;i++){
        switch(child_pids[i] = fork()){
            case -1:
                perror("fork porto");
                break;
            case 0:
                argv_figli[0] = strcpy(argv_figli[0], "./porto");
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
                argv_figli[0] = strcpy(argv_figli[0], "./nave");
				sprintf(argv_figli[1], "%d", (i));
				execve("./nave", argv_figli, NULL);
                perror("execve nave");
                exit(EXIT_FAILURE);
                break;
            default:
                break;
        }
    }
    
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);

    /* Passo i PIDS sulla FIFO per il METEO */
    
    /*write(fd_fifo_pids, child_pids, sizeof(int)*(SO_NAVI+SO_PORTI+1));*/
    for(i = 0; i < SO_PORTI + SO_NAVI; i++){
        write(fd_fifo_pids, &(child_pids[i]), sizeof(int));
    }

    /* Fine settaggio argv_figli e creazione dei processi.
     * Inizio attesa di sincronizzazione e partenza del loop di simulazione. */
    for(i=0;i<SO_PORTI;i++){ mask_porti_generanti[i] = 0; }
    CAST_DUMP(vptr_shm_dump)->porti_generanti = rand() % SO_PORTI + 1;
    for(i=0;i<CAST_DUMP(vptr_shm_dump)->porti_generanti;){
        if(mask_porti_generanti[i]==0){ mask_porti_generanti[i] = 1; i++; }
    }
    fprintf(stderr, "%s %d %d\n", __FILE__, __LINE__, getpid());

    sem_wait_zero(id_semaforo_gestione, 0);
    fprintf(stderr, "%s %d\n", __FILE__, __LINE__);

    for(i=0;i<SO_PORTI;i++){
        if(mask_porti_generanti[i] == 1){
            kill(child_pids[i], SIGUSR2);
        }
    }
    stampa_dump(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&(sa.sa_mask));
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    continua_simulazione = 1;
    do{
        alarm(1);
        if(errno && errno != EINTR)
            printf("\nErrno = %d dopo alarm: %s\n", errno, strerror(errno));
        if(!(continua_simulazione = controlla_mercato(vptr_shm_mercato, vptr_shm_dump, PARAMETRO))){
            printf("\nMASTER: Termino la simulazione per mancanza di offerte e/o di richieste!\n");
        }
        CAST_DUMP(vptr_shm_dump)->porti_generanti = rand() % SO_PORTI + 1;
        /* MANDARE SEGNALI A PORTI CASUALI */
        pause();
    } while(((int)(CAST_DUMP(vptr_shm_dump)->data) < SO_DAYS) && continua_simulazione);
    
    close(fd_fifo_demone);
    close(fd_fifo_pids);
    unlink(NOME_FIFO);
    unlink(FIFO_PIDS);
    kill(demone_pid, SIGUSR1);
    kill(meteo_pid, SIGUSR1);
    for(i = 0; i < SO_NAVI+SO_PORTI; i++){
        printf("MASTER: ammazzo il figlio %d\n", child_pids[i]);
        kill(child_pids[i], SIGUSR1);
    }
    
    i=0;
    while((child_pid = wait(&status)) != -1){
        printf("Terminato figlio %d status %d\n", child_pid, WEXITSTATUS(status));
    }
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
    distruggi_risorse(id_shm_mercato, id_shm_dettagli_lotti, id_shm_posizioni_porti, id_shm_dump, id_coda_richieste, id_coda_meteo);
    distruggi_semafori(id_semaforo_mercato, id_semaforo_dump, id_semaforo_banchine, id_semaforo_gestione);
    /* sono da liberare child_pids, ogni argv_figli[i] meno l'ultimo che è null,
     *  e argv_figli stesso => tot=(QNT_PARAMETRI + 2))+1; */
    
    free_ptrs(child_pids, argv_figli, argv_demone, argv_meteo);
    exit(EXIT_SUCCESS);
}

void signal_handler(int signo){
    int i;
    switch(signo){
        case SIGALRM:
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
            sem_release(id_semaforo_gestione, 1);
            /* il meteo eseguirà la reserve. */
            break;
        case SIGINT:
            printf("TERMINAZIONE! Simulazione completata per mancanza di navi.\n");
            continua_simulazione = 0;
            stampa_terminazione(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
            fprintf(stderr, "\x1b[%dF\x1b[0J", 1);
            fprintf(stderr, "Simulazione completata ^_^\n");
            break;
        default: 
            perror("MASTER: giunto segnale diverso da SIGALRM!");
            continua_simulazione = 0;
    }
}