#include "definitions.h" /* contiene le altre #include */
#include "my_lib.h"
#include "merci_lib.h"
#include "dump_lib.h"
#include "sem_lib.h"


/* id del set di semafori BANCHINE */
 int id_semaforo_banchine;
/* id del set di semafori DUMP */
 int id_semaforo_dump;
/* id del set di semafori GESTIONE */
 int id_semaforo_gestione;
/* id del set di semafori MERCATO */
 int id_semaforo_mercato;

/* Puntatore del DUMP*/
 dump *ptr_dump;
/* id del DUMP*/
 int id_dump;

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];

int inizializzaSemaforoDump();

int main(int argc, char *argv[]) {
	int *childs;
	int NUM_RIGA_FILE, i, j, file_config_char, status = 0;
	/* id delle risorse ipc create */
	int id_mercato, id_posizioni, id_lotti;
	int child_pid;
	FILE *file_config;
	char *str = (char *)malloc(MAX_FILE_STR_LEN);
	merce *ptr_lotti, *ptr_mercato;
	/* Vi ci si riferirà come "posizioni_p+i", dove i definito in [0,SO_PORTI-1] */
	point *ptr_posizioni;
	char *argv_figli[QNT_PARAMETRI + 3]; /* null terminated */

	/* fine definizioni */
	if (argc != 2){
		fprintf(stderr, "Ri-eseguire il makefile con il parametro: var=[NUM_RIGA_FILE-1]\n");
		exit(EXIT_FAILURE);
	}
	NUM_RIGA_FILE = atoi(argv[1]);
	if (NUM_RIGA_FILE < 1){
		ERROR("parametro inserito in entrata (%d) non adatto")
		fprintf(stderr, "NUM_RIGA_FILE: %d\n", NUM_RIGA_FILE);
		exit(EXIT_FAILURE);
	}
	file_config = fopen("config.txt", "r");
	TEST_ERROR

	/* mangia -con fgetc()- tutti i caratteri fino a quello da cui iniziare */
	for (i = 0; i < NUM_RIGA_FILE;) {
		if (file_config_char = fgetc(file_config) == '\n')
			i++;
		else if (file_config_char == EOF){
			ERROR("nella ricerca dell'indice passato come parametro")
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}

	/* inserisce su parametro[i] i valori del file */
	for (i = 0; i < QNT_PARAMETRI; i++) {
		if (fscanf(file_config, "%d", &PARAMETRO[i]) != 1){
			ERROR("nella lettura di un parametro da file")
			fprintf(stderr, "Indice dell'errore su file: %d\n", i);
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}
	fclose(file_config);
	STAMPA_PARAMETRI
	
	childs = (int *)malloc((SO_NAVI + SO_PORTI) * sizeof(int));
	
	#ifdef DEBUG
		printf("Dump creato con id = %d\n", dump_id);
	#endif

	#ifdef DEBUG
		printf("Memoria mercato creata con id = %d\n", porti_id);
	#endif

	/* creazione delle strutture IPC necessarie */
	printf("SHM POSIZIONI: %d\n", id_posizioni = alloca_shm_posizioni(SO_PORTI));
	TEST_ERROR
	printf("SHM mercato: %d\n", id_mercato =  alloca_shm_mercato(SO_PORTI, SO_MERCI));
	TEST_ERROR
	printf("SHM LOTTI: %d\n", id_lotti = alloca_shm_lotti(SO_MERCI));
	TEST_ERROR
	printf("SHM DUMP: %d\n", id_dump = alloca_shm_dump(SO_MERCI, SO_PORTI));
	TEST_ERROR
	printf("QUEUE (-1 == fail): %d\n", alloca_coda_richieste());
	TEST_ERROR
	printf("SET SEM BANCHINE (-1 == fail): %d\n", id_semaforo_banchine = crea_semaforo_banchine(SO_PORTI));
	TEST_ERROR
	printf("SET SEM DUMP (-1 == fail): %d\n", id_semaforo_dump = crea_semaforo_dump(SO_MERCI));
	TEST_ERROR
	printf("SET SEM GESTIONE (-1 == fail): %d\n", id_semaforo_gestione = crea_semaforo_gestione());
	TEST_ERROR
	printf("SET SEM MERCATO (-1 == fail): %d\n", id_semaforo_mercato = crea_semaforo_mercato(SO_PORTI));
	TEST_ERROR
	printf("SET SEM DUMP (-1 == fail): %d\n", inizializzaSemaforoDump());
	ptr_posizioni = aggancia_shm(id_posizioni);
	ptr_mercato = aggancia_shm(id_mercato);
	ptr_lotti = aggancia_shm(id_lotti);
	ptr_dump = aggancia_shm(id_dump);
	ptr_dump->merce_dump_ptr = (merce_dump*)ptr_dump;
	ptr_dump->porto_dump_ptr = (porto_dump*)(((merce_dump*)ptr_dump)+SO_MERCI);

	/* creazione delle posizioni e aggiunta alla memoria dedicata */
	/* I primi quattro porti creati agli angoli */
	ptr_posizioni[0].x = ptr_posizioni[0].y = ptr_posizioni[1].y = ptr_posizioni[2].x = 0;
	ptr_posizioni[1].x = ptr_posizioni[2].y = ptr_posizioni[3].x = ptr_posizioni[3].y = SO_LATO;
	srand(time(NULL));
	for (i = 4; i < SO_PORTI; i++) {
		int found = 0;
		point tmp = generate_rand_point(SO_LATO);
		for (j = 0; j < i && !found; j++) {
			if (equals(tmp.x, (ptr_posizioni + j)->x) && (equals(tmp.y, (ptr_posizioni + j)->y))) {
				found = 1;
			}
		}
		if (found) {
			i--;
		} else {
			(ptr_posizioni + i)->x = tmp.x;
			(ptr_posizioni + i)->y = tmp.y;
		}
	}
	
	for (i = 0; i < SO_MERCI; i++){
		*(ptr_lotti + i) = setUpLotto(SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA);
		/*debug
			printf("Merce %d: val = %d, exp = %d\n", i, (dettagliLotti + i)->val, (dettagliLotti + i)->exp);
		 */
	}

	/* definizione dell'argv dei figli */
	argv_figli[0] = (char *)malloc(MAX_STR_LEN);
	argv_figli[1] = (char *)malloc(MAX_STR_LEN);
	for (i = 0; i < QNT_PARAMETRI; i++){
		argv_figli[i+2] = (char *)malloc(MAX_STR_LEN);
		sprintf(argv_figli[i+2], "%d", PARAMETRO[i]);
		TEST_ERROR
	}
	argv_figli[QNT_PARAMETRI + 2] = NULL;

	/* creazione porti e navi... */
	for (i = 0; i < (SO_PORTI + SO_NAVI); i++) {
		switch (childs[i] = fork()){
		case -1:
			TEST_ERROR
			break;
		case 0: /* proc figlio */
			if (i < SO_PORTI){
				argv_figli[0] = "./porto";
				sprintf(argv_figli[1], "%d", i);
				execve("./porto", argv_figli, NULL);
				/* RISOLTO IL 16 GENNAIO 2023*/
				ERROR("nella execve")
			} else {
				argv_figli[0] = "./nave";
				sprintf(argv_figli[1], "%d", (i - SO_PORTI));
				execve("./nave", argv_figli, NULL);
				ERROR("nella execve")
			}
			return -1; /* caso in cui non funzioni la exec */
			break;
		default:
			break;
		}
	}
	/* avvio del timer della simulazione (+ sincronizzazione) */

	/* settare il semaforo di preparazione iniziale (gestione: SO_NAVI + SO_PORTI)*/
	if(sem_set_val(id_semaforo_gestione, 0, SO_NAVI+SO_PORTI) == -1){
		ERROR("nel MASTER causato dal sem_set_val()")
		TEST_ERROR
	}
	/*deve aspettare il semaforo di preparazione (gestione), che passi da X a zero */
	if(sem_waitforzero(id_semaforo_gestione, 0) == -1){
		ERROR("nel MASTER causato dal sem_waitforzero()")
		TEST_ERROR
	}
	
	/* dopodiché crea il tempo (salviamo su dump un campo int, 
				aggiornato dal mastere visibile anhe alle navi,
		 		dopo la nanosleep)
		si entra in un ciclo che termina:
		- dopo SO_DAYS
		- se finiscono le richieste
		- se finiscono le offerte
	*/
	sleep(4);
	
	for(i=0;i<(SO_PORTI+SO_NAVI);i++){
		kill(childs[i], SIGUSR1);
	}
	sleep(5);
	for(i=0;i<(SO_PORTI+SO_NAVI);i++){
		kill(childs[i], SIGUSR2);
	}


	/* WAIT di terminazione dei figli */
	i = 0;
	while ((child_pid = wait(&status)) != -1) {
		printf("PARENT: PID=%d. Got info of child with PID=%d, status=%d\n", getpid(), child_pid, WEXITSTATUS(status));
		i++;
	}
	/* in questo caso deve segnalare errno = ECHILD */
	if (errno != ECHILD) {
		ERROR("nell'attesa della terminazione dei processi figli.\nerrno != ECHILD")
		TEST_ERROR
	}
	else {
		printf("Chiusura di tutti i %d processi effettuata.\nInizio deallocazione risorse IPC.\n", i);
	}
	/* A fine while la wait resituirà -1 e setterà errno a 10 (No child processes)*/
	if(errno == ECHILD) errno = 0;
	



	/* STAMPA MERCATO */
	/*merce (*mercato)[SO_MERCI] = aggancia_shm_mercato();*/

	for(i=0;i<SO_PORTI;i++){
		printf("Porto %d\n", i);
		for(j=0;j<SO_MERCI;j++){
			printf("Merce %d nlotti: %d | scadenza: %d\n", j, (*(ptr_mercato+i+j)).val , ((*(ptr_mercato+i+j)).exp));
		}
	}
	richiesta rich;
	for(i=0;i<SO_PORTI;i++){
		rich = accettaRichiesta(i);
		while(rich.mtext.indicemerce!=-1){
			printf("Porto %ld richiesto %d lotti di %d merce.\n", rich.mtype, rich.mtext.nlotti, rich.mtext.indicemerce);
			rich = accettaRichiesta(i);
		}
	}
	
	
	/* rimozione risorse IPC */
	printf("sganciaMercato() = %d\n", sgancia_shm(ptr_mercato));
	printf("distruggiMercato() = %d\n", distruggi_shm(id_mercato));

	printf("sganciaPosizioni() = %d\n", sgancia_shm(ptr_posizioni));
	printf("distruggiPosizioni() = %d\n", distruggi_shm(id_posizioni));

	printf("sganciaLotti() = %d\n", sgancia_shm(ptr_lotti));
	printf("distruggiShmLotti() = %d\n", distruggi_shm(id_lotti));
	
	printf("distruggiCoda() = %d\n", distruggi_coda());

	printf("sganciaMemoriaDump() = %d\n", sgancia_shm(ptr_dump));
	printf("distruggiMemoriaDump() =%d\n", distruggi_shm(id_dump));
	
	printf("distruggiSemaforoBanchine()= %d\n", distruggi_semaforo(id_semaforo_banchine));
	printf("distruggiSemaforoDump()= %d\n", distruggi_semaforo(id_semaforo_dump));
	printf("distruggiSemaforoGestione()= %d\n", distruggi_semaforo(id_semaforo_gestione));
	printf("distruggiSemaforoMercato()= %d\n", distruggi_semaforo(id_semaforo_mercato));

	exit(EXIT_SUCCESS);
}

int inizializzaSemaforoDump(){
	return sem_setall(SO_MERCI+1,1,id_semaforo_dump);
}