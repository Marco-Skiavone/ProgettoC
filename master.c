#include "definitions.h"	/* contiene le altre #include */
#include "my_lib.h"

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];


int main(int argc, char *argv[]){
	int NUM_RIGA_FILE, i, file_config_char, status = 0;
	/* id delle risorse ipc create */
	int dump_id, mercato_id, posizioni_id, coda_richieste_id, sem_banchine_id;
	int child_pid;
	FILE *file_config;
	char *str = (char *) malloc(MAX_FILE_STR_LEN);
	char *dump_p = (char *) malloc(SIZE_DUMP);
	char *mercato_p = (char *) malloc(SIZE_MERCATO);
	char *posizioni_p = (char *) malloc(SIZE_POSIZIONI);
	char *argv_figli[QNT_PARAMETRI+2]; /* null terminated */
	/* fine definizioni di tipi */
	if(argc != 2){
		fprintf(stderr, "Ri-eseguire con il parametro: var=[NUM_RIGA_FILE]\n");
		exit(EXIT_FAILURE);
	}
	NUM_RIGA_FILE = atoi(argv[1]);
	if(NUM_RIGA_FILE < 1){
		ERROR("parametro inserito in entrata (%d) non adatto")
		fprintf(stderr, "NUM_RIGA_FILE: %d\n", NUM_RIGA_FILE);
		exit(EXIT_FAILURE);
	}

	file_config = fopen("config.txt", "r");
	TEST_ERROR
	
	/* mangia -con fgetc()- tutti i caratteri fino a quello da cui iniziare */
	for(i = 0; i < NUM_RIGA_FILE;){
		if(file_config_char = fgetc(file_config) == '\n')
			i++;
		else if(file_config_char == EOF){
			ERROR("nella ricerca dell'indice passato come parametro")
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}
	
	/* inserisce su parametro[i] i valori del file */
	for(i = 0; i < QNT_PARAMETRI; i++){
		if(fscanf(file_config, "%d", &PARAMETRO[i]) != 1){
			ERROR("nella lettura di un parametro da file")
			fprintf(stderr, "Indice dell'errore su file: %d\n", i);
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}

	printf("Parametri:\nSO_NAVI | SO_PORTI | SO_MERCI | SO_SIZE | SO_MIN_VITA | SO_MAX_VITA | ");
	printf("SO_LATO | SO_SPEED | SO_CAPACITY | SO_BANCHINE | SO_FILL | SO_LOADSPEED | SO_DAYS\n");
	for(i = 0; i < QNT_PARAMETRI; i++){
		printf(" %d ", PARAMETRO[i]);
		if(i != QNT_PARAMETRI-1)
			printf("|");
		else printf("\n");
	}
	
	/* creazione della shm di dump */
	dump_id = shmget(KEY_DUMP, SIZE_DUMP, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	dump_p = shmat(dump_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Dump creato con id = %d\n", dump_id);
	#endif

	/* creazione della shm "mercato" */
	mercato_id = shmget(KEY_MERCATO, SIZE_MERCATO, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	mercato_p = shmat(mercato_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Memoria mercato creata con id = %d\n", porti_id);
	#endif

	/* creazione della shm "posizioni" */
	posizioni_id = shmget(KEY_POSIZIONI, SIZE_POSIZIONI, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	posizioni_p = shmat(posizioni_id, NULL, 0);
	#ifdef DEBUG
		printf("Memoria posizioni creata con id = %d\n", posizioni_id);
	#endif

	/* creazione coda richieste */
	coda_richieste_id = msgget(KEY_CODA_RICHIESTE, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	#ifdef DEBUG
		printf("Coda di richieste (MSG_Q) creata con id = %d\n", coda_richieste_id);
	#endif

	/* creazione del set di semafori "banchine dei porti" */
	sem_banchine_id = semget(KEY_BANCHINE_SEM, PARAMETRO[SO_PORTI], IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	#ifdef DEBUG
		printf("Set di semafori delle banchine creato con id = %d\n", sem_banchine_id);
	#endif

	/* creazione delle posizioni e aggiunta alla memoria dedicata */


	/* definizione dell'argv dei figli */
	*argv_figli = (char *) malloc(MAX_STR_LEN*(QNT_PARAMETRI+1));
	for(i = 1; i < QNT_PARAMETRI+1; i++){
		argv_figli[i] = (char *) malloc(MAX_STR_LEN); 
		sprintf(argv_figli[i], "%d", PARAMETRO[i]);
	}
	argv_figli[QNT_PARAMETRI+1] = NULL;

	/* creazione porti e navi... */
	for(i = 0; i < (PARAMETRO[SO_PORTI]+PARAMETRO[SO_NAVI]); i++){
		switch(fork()){
			case -1:
				TEST_ERROR
				break;
			case 0:	/* proc figlio */
				if(i < PARAMETRO[SO_PORTI]){
					sprintf(argv_figli[0], "%d", i);
					execl("porto", argv_figli);
				} else {
					sprintf(argv_figli[0], "%d", (i-PARAMETRO[SO_PORTI]));
					execl("nave", argv_figli);
				}
				break;
			default:
				break;
		}
	}
	/* avvio del timer della simulazione (+ sincronizzazione) */

	
	/* DUMP */

	/* WAIT di terminazione dei figli */
	while ((child_pid = wait(&status)) != -1) {
		printf("PARENT: PID=%d. Got info of child with PID=%d, status=%d\n", getpid(), child_pid, WEXITSTATUS(status));
	}
	/* in questo caso deve segnalare errno = ECHILD */
	if(errno != ECHILD){
		ERROR("nell'attesa della terminazione dei processi figli.\nerrno != ECHILD")
		TEST_ERROR
	} else 
		printf("Chiusura di tutti i processi effettuata.\nInizio deallocazione risorse IPC.\n");

	/* de-allocazione risorse IPC */
	if(semctl(sem_banchine_id, 0, IPC_RMID) == 0)
		printf("Array banchine deallocato.\n");
	else {
		ERROR("nella deallocazione dell'array banchine")
	}
	if(shmctl(dump_id, 0, IPC_RMID) == 0)
		printf("Memoria dump deallocata.\n");
	else {
		ERROR("nella deallocazione della memoria dump")
	}
	if(shmctl(mercato_id, 0, IPC_RMID) == 0)
		printf("Memoria mercato deallocata.\n");
	else {
		ERROR("nella deallocazione della memoria mercato")
	}
	if(shmctl(posizioni_id, 0, IPC_RMID) == 0)
		printf("Memoria delle posizioni deallocata.\n");
	else {
		ERROR("nella deallocazione della memoria delle posizioni")
	}
	if(shmctl(coda_richieste_id, 0, IPC_RMID) == 0)
		printf("Coda delle richieste deallocata.\n");
	else {
		ERROR("nella deallocazione della coda delle richieste")
	}
	exit(EXIT_SUCCESS);
}