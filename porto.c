#include "definitions.h"
#include "my_lib.h"
#include "merci_lib.h"
#include "sem_lib.h"
#include "dump_lib.h"

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
int DATA = 0;
merce *vecchioDump;
int indice;
merce *ptr_lotti;
merce *ptr_mercato;

void inizializza_banchine(int indice);

void sigusr1_handler(int signum);

void sigusr2_handler(int signum);

int main(int argc, char *argv[]){
	int i;
	int id_mercato, id_lotti;
	richiesta rich;
	struct sigaction sigusr1_sa, sigusr2_sa;

	sigusr1_sa.sa_handler = sigusr1_handler_porto;
	sigusr2_sa.sa_handler = sigusr2_handler_porto;

	sigemptyset(&sigusr1_sa.sa_mask);
	sigaction(SIGUSR1, &sigusr1_sa, NULL);

	sigemptyset(&sigusr2_sa.sa_mask);
	sigaction(SIGUSR2, &sigusr2_sa, NULL);

	/*1 = # porto (per array banchine), #[QNT_PARAMETRI] parametri */
	if (argc != (2 + QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri al porto")
		exit(99);
	}
	indice = atoi(argv[1]);
	TEST_ERROR
	for (i = 2; i < argc; i++){
		PARAMETRO[i - 2] = atoi(argv[i]);
		TEST_ERROR
	}
	vecchioDump = malloc(sizeof(merce) * SO_MERCI);

	/*
	fprintf(stderr, "PORTO %d: parametri letti.\n", getpid());
	STAMPA_PARAMETRI
	printf("indice: %d\n", indice);
	*/

	srand(getpid());
	set_coda_richieste();
	id_dump = set_shm_dump(SO_MERCI, SO_PORTI);
	ptr_dump = aggancia_shm(id_dump);
	id_mercato = set_shm_mercato(SO_PORTI, SO_MERCI);
	id_lotti = set_shm_lotti(SO_MERCI);
	ptr_mercato = aggancia_shm(id_mercato);
	ptr_lotti = aggancia_shm(id_lotti);
	id_semaforo_banchine = trova_semaforo_banchine(SO_PORTI);
	id_semaforo_dump = trova_semaforo_dump(SO_MERCI);
	id_semaforo_gestione = trova_semaforo_gestione();
	id_semaforo_mercato = trova_semaforo_mercato(SO_PORTI);

	spawnMerciPorti((SO_FILL / SO_PORTI), SO_MERCI, indice, ptr_mercato, ptr_lotti);
	TEST_ERROR

	inizializza_banchine(indice);
	TEST_ERROR
	/* MANDA RICHIESTA E INIZIALIZZA VETTORE VECCHIO DUMP*/
	for (i = 0; i < SO_MERCI; i++){
		if (((merce(*)[SO_MERCI])ptr_mercato)[indice][i].val < 0){
			rich.mtype = indice;
			rich.mtext.nlotti = -((merce(*)[SO_MERCI])ptr_mercato)[indice][i].val;
			rich.mtext.indicemerce = i;
			inviaRichiesta(rich);

			vecchioDump[i] = ((merce(*)[SO_MERCI])ptr_mercato)[indice][i];
		}
		else {
			vecchioDump[i] = ((merce(*)[SO_MERCI])ptr_mercato)[indice][i];
		}
	}
	/* INIZIO CICLO DI DUMP */
	do {
		if (sem_reserve(id_semaforo_gestione, 0) == -1) {
			ERROR("nel PORTO causato dal sem_reserve()")
			TEST_ERROR
		}
		if (sem_waitforzero(id_semaforo_gestione, 0) == -1) {
			ERROR("nel PORTO causato dal sem_waitforzero()")
			TEST_ERROR
		}
		pause();
	} while (DATA != (SO_DAYS+1));
	exit(199);
}

void inizializza_banchine(int i){
	int nbanchine;
	nbanchine = rand() % SO_BANCHINE + 1;
	/*printf("PORTO %d: id_sem_banchine = %d\n", i, get_id_sem_banchine());*/
	sem_set_val(id_semaforo_banchine, i, nbanchine);
}

void sigusr1_handler_porto(int signum){

	int i;
	merce(*ptr)[SO_MERCI] = (merce(*)[SO_MERCI])ptr_mercato;

	/*printf("dumb pointer: %p\n", ptr);*/
	for (i = 0; i < SO_MERCI; i++){
		if (vecchioDump[i].val < ptr[indice][i].val){
			((porto_dump *)ptr_dump)[indice].mercericevuta += (ptr[indice][i].val - vecchioDump[i].val);
		}
		if (vecchioDump[i].val > ptr[indice][i].val){ /* forse da errore */
			((porto_dump *)ptr_dump)[indice].mercespedita += (vecchioDump[i].val - ptr[indice][i].val);
		}

		if (ptr[indice][i].exp < DATA){ /*va messo < al posto di > */
			if (sem_reserve(id_semaforo_dump, i) == 0){
				TEST_ERROR
				((merce_dump *)ptr_dump)[i].scaduta_in_porto += ptr[indice][i].val;
				sem_release(id_semaforo_dump, i);
				TEST_ERROR
			}
		}
	}
	DATA++;
}

void sigusr2_handler_porto(int signum){
	/*printf("porto 1:\n");*/
	sgancia_shm(ptr_dump);
	/*printf("porto 2:\n");*/
	sgancia_shm(ptr_mercato);
	/*printf("porto 3:\n");*/
	sgancia_shm(ptr_lotti);
	/*printf("porto 4:\n");*/

	free(vecchioDump);
	exit(10 + indice);
}