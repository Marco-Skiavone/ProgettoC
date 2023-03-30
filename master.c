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

// int DATA;
int *child_pids;

int PARAMETRO[QNT_PARAMETRI];
int equals(double x, double y);
point generate_random_point(int lato);
void generate_positions(double lato, point* posizioni_porti);

void alloca_risorse();
void sgancia_e_distruggi_risorse();

/* inizializza i valori del dump, tra cui i puntatori, chiamato dal master a inizio simulazione */
void inizializza_dump();

void setUpLotto(merce* ptr_dettagli_lotti, int nmerci, int so_size, int so_min_vita, int so_max_vita);

void signal_handler(int signo);

int main(int argc, char* argv[]){
    int i, j, k;
    int n_righe_file, file_config_char;
    
    int continua_simulazione;

    int child_pid, status;
    FILE *file_config;
    richiesta r;
    char *argv_figli[QNT_PARAMETRI + 3];
    setbuf(stdout, NULL); /* unbufferizza stdout */
    clearLog();
    srand(time(NULL));
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

    alloca_risorse();
    inizializza_dump();
    CAST_DUMP(vptr_shm_dump)->data = 0;
    generate_positions(SO_LATO, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti));
    
    /*
    for(i=0;i<SO_PORTI;i++){
        printf("Porto %d - x: %f y: %f\n", i, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[i].x, CAST_POSIZIONI_PORTI(vptr_shm_posizioni_porti)[i].y);
    }
    */
    
    setUpLotto(CAST_DETTAGLI_LOTTI(vptr_shm_dettagli_lotti), SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA);
    
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
    printf("Set id_semaforo_gestione a %d + %d = %d\n", SO_PORTI, SO_NAVI, sem_get_val(id_semaforo_gestione, 0));

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

    struct sigaction sa_alrm;
    sa_alrm.sa_handler = signal_handler;
    sa_alrm.sa_flags = 0;
    sigemptyset(&(sa_alrm.sa_mask));
    sigaction(SIGALRM, &sa_alrm, NULL);
    // DATA = 0;
    
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
    
    /*
    */
    for(i=0;i<SO_PORTI;i++){
        printf("Porto %d\n", i);
        for(j=0;j<SO_MERCI;j++){
            printf("Merce %d nlotti %d scadenza %d\n", j, ((merce(*)[SO_MERCI])vptr_shm_mercato)[i][j].val, ((merce(*)[SO_MERCI])vptr_shm_mercato)[i][j].exp);
        }
    }
    printf("\n__________________________ \n\n");

    i = 0;
    do {
        r = accetta_richiesta(i, id_coda_richieste);
        if(r.mtext.indicemerce != -1)
            printf("Porto %ld merce %d nlotti %d\n", r.mtype, r.mtext.indicemerce, r.mtext.nlotti);
        else if(i < SO_PORTI)
            i++;
    } while (r.mtext.indicemerce != -1 || i < SO_PORTI);
    printf("\n__________________________ \n\n");


    sgancia_e_distruggi_risorse();

}

void setUpLotto(merce* ptr_dettagli_lotti, int nmerci, int so_size, int so_min_vita, int so_max_vita){
    int i;
    ptr_dettagli_lotti[0].val = 1;
    ptr_dettagli_lotti[0].exp = so_min_vita + (rand() % (so_max_vita - so_min_vita));
    for(i=1;i<nmerci;i++){
        ptr_dettagli_lotti[i].val = (rand() & so_size) + 1;
        ptr_dettagli_lotti[i].exp = so_min_vita + (rand() % (so_max_vita - so_min_vita));
    }
}

int equals(double x, double y){
	if(x > y)
		return (x-y) < TOLLERANZA ? 1: 0;
	else 
		return (y-x) < TOLLERANZA ? 1 : 0;
}

point generate_random_point(int lato) {
    int mant, p_intera;
    point p;
    p_intera = mant = rand()%lato;

	p.x = ((double)mant/lato) + (p_intera*getppid()%lato);
	p_intera = mant = rand()%lato;
	p.y = ((double)mant/lato) + ((p_intera*getppid()%lato));
    return p;
}

void generate_positions(double lato, point* posizioni_porti) {
    int i;
    int j;
    point p;
    int found;
    posizioni_porti[0].x = 0;       posizioni_porti[0].y = 0;
    posizioni_porti[1].x = lato;    posizioni_porti[1].y = 0;
    posizioni_porti[2].x = lato;    posizioni_porti[2].y = lato;
    posizioni_porti[3].x = 0;       posizioni_porti[3].y = lato;

    for (i = 4; i < SO_PORTI; i++) {
        p = generate_random_point(lato);
        found = 0;
        for (j = 0; j < i && !found; j++) {
			if (equals(p.x, posizioni_porti[j].x) && (equals(p.y, posizioni_porti[j].y))) {
				found = 1;
			}
		}
		if (found) {
			i--;
		} else {
			posizioni_porti[i].x = p.x;
            posizioni_porti[i].y = p.y;
		}
    }
}



void alloca_risorse(){
    int i;

    printf("SHAREDM_MERCATO: %d\n", id_shm_mercato = alloca_shm(CHIAVE_SHAREDM_MERCATO, SIZE_SHAREDM_MERCATO));
    vptr_shm_mercato = aggancia_shm(id_shm_mercato);

    
    printf("SHAREDM_DETTAGLI_LOTTI: %d\n", id_shm_dettagli_lotti = alloca_shm(CHIAVE_SHAREDM_DETTAGLI_LOTTI, SIZE_SHAREDM_DETTAGLI_LOTTI));
    vptr_shm_dettagli_lotti = aggancia_shm(id_shm_dettagli_lotti);

    printf("SHAREDM_POSIZIONI_PORTI: %d\n", id_shm_posizioni_porti = alloca_shm(CHIAVE_SHAREDM_POSIZIONI_PORTI, SIZE_SHAREDM_POSIZIONI_PORTI));
    vptr_shm_posizioni_porti = aggancia_shm(id_shm_posizioni_porti);

    printf("SHAREDM_DUMP: %d\n",id_shm_dump =  alloca_shm(CHIAVE_SHAREDM_DUMP, SIZE_SHAREDM_DUMP));
    vptr_shm_dump = aggancia_shm(id_shm_dump);


    printf("SEM_CREATE_MERCATO: %d\n", id_semaforo_mercato = sem_create(CHIAVE_SEM_MERCATO, SO_PORTI));
    for(i=0;i<SO_PORTI;i++){
        sem_set_val(id_semaforo_mercato, i, 1);
    }

    printf("SEM_CREATE_GESTIONE: %d\n", id_semaforo_gestione = sem_create(CHIAVE_SEM_GESTIONE, 2));

    printf("SEM_CREATE_BANCHINE: %d\n", id_semaforo_banchine = sem_create(CHIAVE_SEM_BANCHINE, SO_PORTI));

    printf("SEM_CREATE_DUMP: %d\n", id_semaforo_dump = sem_create(CHIAVE_SEM_DUMP, 2));

    printf("CODA RICHIESTE: %d\n", id_coda_richieste = set_coda_richieste(CHIAVE_CODA));

    printf("\n__________________________ \n\n");
}

void sgancia_e_distruggi_risorse(){

    printf("SGANCIA_SHAREDM_MERCATO\n"); sgancia_shm(vptr_shm_mercato);
    printf("SGANCIA_SHAREDM_DETTAGLI_LOTTI\n"); sgancia_shm(vptr_shm_dettagli_lotti);
    printf("SGANCIA_SHAREDM_POSIZIONI_PORTI\n"); sgancia_shm(vptr_shm_posizioni_porti);
    printf("SGANCIA_SHAREDM_DUMP\n"); sgancia_shm(vptr_shm_dump);

    printf("DISTRUGGI_SHAREDM_MERCATO\n"); distruggi_shm(id_shm_mercato);
    printf("DISTRUGGI_SHAREDM_DETTAGLI_LOTTI\n"); distruggi_shm(id_shm_dettagli_lotti);
    printf("DISTRUGGI_SHAREDM_POSIZIONI_PORTI\n"); distruggi_shm(id_shm_posizioni_porti);
    printf("DISTRUGGI_SHAREDM_DUMP\n"); distruggi_shm(id_shm_dump);

    printf("DISTRUGGI_SEM_MERCATO\n"); sem_destroy(id_semaforo_mercato);
    printf("DISTRUGGI_SEM_DUMP\n"); sem_destroy(id_semaforo_dump);
    printf("DISTRUGGI_SEM_BANCHINE\n"); sem_destroy(id_semaforo_banchine);
    printf("DISTRUGGI_SEM_GESTIONE\n"); sem_destroy(id_semaforo_gestione);
    printf("DISTRUGGI_CODA_RICHIESTE\n"); destroy_coda(id_coda_richieste);

    printf("\n__________________________ \n\n");
}

void inizializza_dump(){
    int i;
    CAST_DUMP(vptr_shm_dump)->data = 0;
    CAST_DUMP(vptr_shm_dump)->merce_dump_ptr = (merce_dump*)(vptr_shm_dump+sizeof(int));/* probabilmente non funziona! ( ma andrebbe davvero inizializzato! )*/
    CAST_DUMP(vptr_shm_dump)->porto_dump_ptr = (porto_dump*)(((merce_dump*) vptr_shm_dump+sizeof(int))+SO_MERCI);

    CAST_TERM_DUMP(vptr_shm_dump).porto_ricevute = 0;
    CAST_TERM_DUMP(vptr_shm_dump).porto_spedite = 0;

    CAST_DUMP(vptr_shm_dump)->nd.navicariche = 0;
    CAST_DUMP(vptr_shm_dump)->nd.naviscariche = SO_NAVI;
    CAST_DUMP(vptr_shm_dump)->nd.naviporto = 0;
    
    for(i = 0; i < SO_MERCI; i++){
        CAST_MERCE_DUMP(vptr_shm_dump)[i].consegnata = 0;
        CAST_MERCE_DUMP(vptr_shm_dump)[i].presente_in_nave = 0;
        CAST_MERCE_DUMP(vptr_shm_dump)[i].presente_in_porto = 0;
        CAST_MERCE_DUMP(vptr_shm_dump)[i].scaduta_in_nave = 0;
        CAST_MERCE_DUMP(vptr_shm_dump)[i].scaduta_in_porto = 0;
    }
    for(i = 0; i < SO_PORTI; i++){
        CAST_PORTO_DUMP(vptr_shm_dump)[i].mercepresente = 0;
        CAST_PORTO_DUMP(vptr_shm_dump)[i].mercericevuta = 0;
        CAST_PORTO_DUMP(vptr_shm_dump)[i].mercespedita = 0;
    }
    /* bzero(CAST_DUMP(vptr_shm_dump)->merce_dump_ptr, sizeof(merce_dump)*SO_MERCI) */
}


void signal_handler(int signo){
    int i;
    switch(signo){
        case SIGALRM:
            if(CAST_DUMP(vptr_shm_dump)->data < SO_DAYS-1){
                CAST_DUMP(vptr_shm_dump)->data++;
                printf("\nMASTER: Passato giorno %d su %d.\n", CAST_DUMP(vptr_shm_dump)->data, SO_DAYS);
                stampa_dump(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
                for(i = 0; i < SO_NAVI+SO_PORTI; i++)
                    { kill(child_pids[i], SIGUSR1);}
            } else {
                CAST_DUMP(vptr_shm_dump)->data++;
                stampa_terminazione(PARAMETRO, vptr_shm_dump, vptr_shm_mercato, id_semaforo_banchine);
            }
            break;
        default: 
            perror("MASTER: giunto segnale diverso da SIGALRM!");
            exit(254);
    }
}