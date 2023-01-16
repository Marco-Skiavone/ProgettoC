#include "definitions.h" /* contiene le altre #include */
#include "my_lib.h"
#include "merci_lib.h"

/* contiene tutti i parametri */
int PARAMETRO[QNT_PARAMETRI];

int main(int argc, char *argv[]) {
	int *childs;
	int NUM_RIGA_FILE, i, j, file_config_char, status = 0;
	/* id delle risorse ipc create */
	int dump_id, mercato_id, posizioni_id, coda_richieste_id, sem_banchine_id;
	int child_pid;
	FILE *file_config;
	char *str = (char *)malloc(MAX_FILE_STR_LEN);
	char *dump_p, *mercato_p;
	/* Vi ci si riferirà come "posizioni_p+i",
	 dove i definito in [0,SO_PORTI-1] */
	point *posizioni_p;
	char *argv_figli[QNT_PARAMETRI + 3]; /* null terminated */

	/* fine definizioni */
	if (argc != 2)
	{
		fprintf(stderr, "Ri-eseguire il makefile con il parametro: var=[NUM_RIGA_FILE-1]\n");
		exit(EXIT_FAILURE);
	}
	NUM_RIGA_FILE = atoi(argv[1]);
	if (NUM_RIGA_FILE < 1)
	{
		ERROR("parametro inserito in entrata (%d) non adatto")
		fprintf(stderr, "NUM_RIGA_FILE: %d\n", NUM_RIGA_FILE);
		exit(EXIT_FAILURE);
	}

	file_config = fopen("config.txt", "r");
	TEST_ERROR

	/* mangia -con fgetc()- tutti i caratteri fino a quello da cui iniziare */
	for (i = 0; i < NUM_RIGA_FILE;)
	{
		if (file_config_char = fgetc(file_config) == '\n')
			i++;
		else if (file_config_char == EOF)
		{
			ERROR("nella ricerca dell'indice passato come parametro")
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}

	/* inserisce su parametro[i] i valori del file */
	for (i = 0; i < QNT_PARAMETRI; i++)
	{
		if (fscanf(file_config, "%d", &PARAMETRO[i]) != 1)
		{
			ERROR("nella lettura di un parametro da file")
			fprintf(stderr, "Indice dell'errore su file: %d\n", i);
			fclose(file_config);
			exit(EXIT_FAILURE);
		}
	}

	printf("Parametri:\nSO_NAVI | SO_PORTI | SO_MERCI | SO_SIZE | SO_MIN_VITA | SO_MAX_VITA | ");
	printf("SO_LATO | SO_SPEED | SO_CAPACITY | SO_BANCHINE | SO_FILL | SO_LOADSPEED | SO_DAYS\n");
	for (i = 0; i < QNT_PARAMETRI; i++)
	{
		printf(" %d ", PARAMETRO[i]);
		if (i != QNT_PARAMETRI - 1)
			printf("|");
		else
			printf("\n");
	}

	childs = (int *)malloc((SO_NAVI + SO_PORTI) * sizeof(int));
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
	posizioni_p = shmat(posizioni_id, NULL, 0);
	TEST_ERROR

	/* creazione delle posizioni e aggiunta alla memoria dedicata */
	printf("Creazione delle posizioni..\n");
	/* I primi quattro porti creati agli angoli */
	posizioni_p[0].x = posizioni_p[0].y = posizioni_p[1].y = posizioni_p[2].x = 0;
	posizioni_p[1].x = posizioni_p[2].y = posizioni_p[3].x = posizioni_p[3].y = SO_LATO;
	srand(time(NULL));
	for (i = 4; i < SO_PORTI; i++)
	{
		int found = 0;
		point tmp = generate_rand_point(SO_LATO);
		for (j = 0; j < i && !found; j++)
		{
			if (equals(tmp.x, (posizioni_p + j)->x) && (equals(tmp.y, (posizioni_p + j)->y)))
			{
				found = 1;
			}
		}
		if (found)
		{
			i--;
		}
		else
		{
			(posizioni_p + i)->x = tmp.x;
			(posizioni_p + i)->y = tmp.y;
		}
	}
	printf("-------------------------------------------------\n");
	
	printf("SHM LOTTI: %d\n", shm_lotti(SO_MERCI));

	printf("QUEUE (-1 fail): %d\n", coda_richieste());

	merce *dettagliLotti = indirizzoDettagliLotti();
	for (i = 0; i < SO_MERCI; i++)
	{
		*(dettagliLotti + i) = setUpLotto(SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA);

		/*debug
		 */
		printf("\n");
		printf("Merce %d: val = %d, exp = %d\n", i, (dettagliLotti + i)->val, (dettagliLotti + i)->exp);
	}

	sganciaMercato();
	sganciaDettagliLotti();

	distruggiMercato();
	distruggiShmDettagliLotti();
	distruggiCoda();

	shmctl(posizioni_id, IPC_RMID, 0);

	/* definizione dell'argv dei figli */
	for (i = 0; i < QNT_PARAMETRI + 2; i++)
	{
		argv_figli[i] = (char *)malloc(MAX_STR_LEN);
		if (i >= 2)
		{
			sprintf(argv_figli[i], "%d", PARAMETRO[i]);
			TEST_ERROR
		}
	}
	argv_figli[QNT_PARAMETRI + 2] = NULL;

	/* creazione porti e navi... */
	for (i = 0; i < (SO_PORTI + SO_NAVI); i++)
	{
		switch (childs[i] = fork())
		{
		case -1:
			TEST_ERROR
			break;
		case 0: /* proc figlio */
			if (i < SO_PORTI)
			{
				argv_figli[0] = "./porto";
				sprintf(argv_figli[1], "%d", i);
				execve("./porto", argv_figli, NULL);
				/* RISOLTO IL 16 GENNAIO 2023*/
				ERROR("nella execve")
			}
			else
			{
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

	/* DUMP */

	/* WAIT di terminazione dei figli */
	i = 0;
	while ((child_pid = wait(&status)) != -1)
	{
		printf("PARENT: PID=%d. Got info of child with PID=%d, status=%d\n", getpid(), child_pid, WEXITSTATUS(status));
		i++;
	}
	/* in questo caso deve segnalare errno = ECHILD */
	if (errno != ECHILD)
	{
		ERROR("nell'attesa della terminazione dei processi figli.\nerrno != ECHILD")
		TEST_ERROR
	}
	else
		printf("Chiusura di tutti i %d processi effettuata.\nInizio deallocazione risorse IPC.\n", i);
	exit(EXIT_SUCCESS);
}