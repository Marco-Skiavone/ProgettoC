#include "definitions.h"	/* contiene le altre #include */
#include "my_lib.h"
#include <sys/stat.h>

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]){
	int NUM_RIGA_FILE, i, j, file_config_char, status = 0;
	/* id delle risorse ipc create */
	int dump_id, mercato_id, posizioni_id, coda_richieste_id, sem_banchine_id;
	int child_pid;
	FILE *file_config;
	char *str = (char *) malloc(MAX_FILE_STR_LEN);
	char *dump_p, *mercato_p;
	/* vi ci si riferirà come "posizioni_p+i",
	 dove i definito in [0,SO_PORTI-1] */
	point *posizioni_p;
	char *argv_figli[QNT_PARAMETRI+3]; /* null terminated */
	/* fine definizioni */
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

	/* creazione della shm di dump *
	dump_id = shmget(KEY_DUMP, SIZE_DUMP, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	dump_p = shmat(dump_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Dump creato con id = %d\n", dump_id);
	#endif

	/* creazione della shm "mercato" *
	mercato_id = shmget(KEY_MERCATO, SIZE_MERCATO, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	mercato_p = shmat(mercato_id, NULL, SHM_RDONLY);
	TEST_ERROR
	#ifdef DEBUG
		printf("Memoria mercato creata con id = %d\n", porti_id);
	#endif*/

	/* creazione della shm "posizioni" */
	posizioni_id = shmget(KEY_POSIZIONI, SIZE_POSIZIONI, IPC_CREAT | IPC_EXCL | PERMESSI);
	TEST_ERROR
	printf("posizioni id: %d\n", posizioni_id);
	posizioni_p = shmat(posizioni_id, NULL, 0);
	printf("posizioni p: %p\n", posizioni_p);
	TEST_ERROR
	#ifdef DEBUG
		printf("Memoria posizioni creata con id = %d\n", posizioni_id);
	#endif

	/* creazione coda richieste *
	coda_richieste_id = msgget(KEY_CODA_RICHIESTE, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	#ifdef DEBUG
		printf("Coda di richieste (MSG_Q) creata con id = %d\n", coda_richieste_id);
	#endif

	/* creazione del set di semafori "banchine dei porti" *
	sem_banchine_id = semget(KEY_BANCHINE_SEM, SO_PORTI, IPC_CREAT | IPC_EXCL | 0666);
	TEST_ERROR
	#ifdef DEBUG
		printf("Set di semafori delle banchine creato con id = %d\n", sem_banchine_id);
	#endif */

	/* creazione delle posizioni e aggiunta alla memoria dedicata */
	
	/*
			CODICE COMMENTATO PRIMA DI PROVARE P++ 

	printf("inizio a creare posizioni!\n");
	posizioni_p[0].x = 0; posizioni_p[0].y = 0;
	posizioni_p[sizeof(point)].x = SO_LATO; posizioni_p[sizeof(point)].y = 0;
	posizioni_p[2*sizeof(point)].x = 0; posizioni_p[2*sizeof(point)].y = SO_LATO;
	posizioni_p[3*sizeof(point)].x = SO_LATO; posizioni_p[3*sizeof(point)].y = SO_LATO;
	printf("prime quattro create!\n");
	for(i = 4*sizeof(point); i < (SO_PORTI*sizeof(point))-1; ){
		int found = 0;
		printf("entro in generate %d!\n", i/sizeof(point));
		point tmp = generate_rand_point(SO_LATO);
		for(j = 0; j < i && !found; j += sizeof(point))
			if(equals(posizioni_p[i].x, posizioni_p[j].x) && equals(posizioni_p[i].y, posizioni_p[j].y))
				found=1;
		if(!found){
			printf("posizione %d generata...\n", i/sizeof(point));
			posizioni_p[i] = tmp;
			i += sizeof(point);
		}
	}

	printf("\nposizioni create:\n");
	
	*/

	printf("Codice Parigi: inizio a creare le posizioni.\n");
	point p0, p1, p2, p3;
	p0.x = 0; p0.y=0;	p1.x = SO_LATO; p1.y = 0;
	p2.x = 0; p2.y = SO_LATO;	p3.x = SO_LATO; p3.y = SO_LATO;

	*(posizioni_p) = p0;
	*(posizioni_p+1) = p1; 
	*(posizioni_p+2) = p2; 
	*(posizioni_p+3) = p3; 

	srand(time(NULL));
	for(i=4;i<SO_PORTI;i++){
		int found = 0;
		point tmp = generate_rand_point(SO_LATO);
		/*printf("tmp.x: %f tmp.y = %f\n", tmp.x, tmp.y);*/
		for(j=0;j<i && !found;j++){
			if(equals(tmp.x, (posizioni_p+j)->x) && (equals(tmp.y, (posizioni_p+j)->y))){
				found = 1;
				
			}
		}
		if(found){
			i--;
		}else{
			(posizioni_p+i)->x = tmp.x;
			(posizioni_p+i)->y = tmp.y;
		}
		found = 0;
		/* printf("\n%d\n", i); */
	}
	printf("-------------------------------------------------\n");
	for(i=0;i<SO_PORTI;i++){
		printf("Porto %d: x = %f, y = %f\n", i, (posizioni_p)->x, (posizioni_p)->y);
		posizioni_p++;
	}
	

	exit(0);

	for(i = 0; i < SO_PORTI*sizeof(point); i += sizeof(point))
		printf("(%f,%f)\n", posizioni_p[i].x, posizioni_p[i].y);
	

	/* definizione dell'argv dei figli */
	*argv_figli = (char *) malloc(MAX_STR_LEN*(QNT_PARAMETRI+1));
	for(i = 2; i < QNT_PARAMETRI+2; i++){
		argv_figli[i] = (char *) malloc(MAX_STR_LEN); 
		sprintf(argv_figli[i], "%d", PARAMETRO[i]);
	}
	argv_figli[QNT_PARAMETRI+2] = NULL;

	/* creazione porti e navi... */
	for(i = 0; i < (SO_PORTI+SO_NAVI); i++){
		switch(fork()){
			case -1:
				TEST_ERROR
				break;
			case 0:	/* proc figlio */
				if(i < SO_PORTI){
					argv_figli[0] = "porto";
					sprintf(argv_figli[1], "%d", i);
					execv("porto", argv_figli);
				} else {
					argv_figli[0] = "nave";
					sprintf(argv_figli[1], "%d", (i-SO_PORTI));
					execv("nave", argv_figli);
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

	


	exit(EXIT_SUCCESS);
}