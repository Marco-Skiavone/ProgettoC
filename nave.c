#include "definitions.h"
#include "my_lib.h"
#include "nave_lib.h"
#include "sem_lib.h"
#include "merci_lib.h"
#include "dump_lib.h"


/* id del set di semafori BANCHINE */
 int id_semaforo_banchine;
/* id del set di semafori DUMP */
 int id_semaforo_dump;
/* id del set di semafori GESTIONE */
 int id_semaforo_gestione;
/* id del set di semafori MERCATO */
 int id_semaforo_mercato;

 int id_mercato, id_posizioni, id_lotti;

/* Puntatore del DUMP*/
 dump *ptr_dump;
/* id del DUMP*/
 int id_dump;

 point *ptr_posizioni;
 merce *ptr_mercato, *ptr_lotti;

int PARAMETRO[QNT_PARAMETRI];
int DATA = 0, indice;
void sigusr1_handler(int signum);
void sigusr2_handler(int signum);

int main(int argc, char *argv[]){
	viaggio v;
	int i, j, k;
	double distanza;
	point position;
	
	struct timespec tempo;
	/* capacità di carico in ton della nave */
	long capacita;
	int nRichieste;
	merce_nave carico[MAX_RICHIESTE];
	richiesta rkst;
	sigset_t maschera;
	struct sigaction sigusr1_sa;
	sigusr1_sa.sa_handler = sigusr1_handler;
	sigusr1_sa.sa_handler = sigusr2_handler;
	sigemptyset(&sigusr1_sa.sa_mask);
	sigemptyset(&maschera);
	sigaddset(&maschera, SIGUSR1);
	sigaction(SIGUSR1, &sigusr1_sa, NULL);
	TEST_ERROR
	/* CODICE PER ATTIVARE LA MASCHERA 
		sigprocmask(SIG_BLOCK, &maschera, NULL);
	*/
	/* CODICE PER DISATTIVARE LA MASCHERA*
		sigprocmask(SIG_UNBLOCK, &maschera, NULL);
	*/
	
	if(argc != (2+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri alla nave")
		exit(99);
	}
	indice = atoi(argv[1]);
	TEST_ERROR
	for(i = 2; i < argc; i++){
		PARAMETRO[i-2] = atoi(argv[i]);
		TEST_ERROR
	}
	
	/* AGGANCIO RISORSE IPC */
	id_dump = set_shm_dump(SO_MERCI, SO_PORTI);
	TEST_ERROR
	id_posizioni = set_shm_posizioni(SO_PORTI);
	id_mercato = set_shm_mercato(SO_PORTI, SO_MERCI);
	id_lotti = set_shm_lotti(SO_MERCI);
	set_coda_richieste();
	/*printf("set posizioni effettuata: %d\n", id_posizioni);
	/* aggancio ai semafori */
	id_semaforo_banchine = trova_semaforo_banchine(SO_PORTI);
	id_semaforo_dump = trova_semaforo_dump(SO_MERCI);
	id_semaforo_gestione = trova_semaforo_gestione();
	id_semaforo_mercato = trova_semaforo_mercato(SO_PORTI);	
	ptr_posizioni = aggancia_shm(id_posizioni);
	ptr_lotti = aggancia_shm(id_lotti);
	ptr_mercato = aggancia_shm(id_mercato);
	ptr_dump = aggancia_shm(id_dump);
	
	/*printf("nave el-8\n");*/
	srand(getpid());
	/* calcolo primo viaggio*/
	position = generate_rand_point(SO_LATO);
	i = calcola_porto_piu_vicino(position, ptr_posizioni, SO_PORTI);
	TEST_ERROR
	distanza = calcola_distanza(position.x, position.y, ptr_posizioni[i].x, ptr_posizioni[i].y);  
	TEST_ERROR
	tempo.tv_sec = distanza / SO_SPEED; 
	tempo.tv_nsec = tempo.tv_sec % SO_SPEED;
	if(nanosleep(&tempo, NULL) != 0){
		printf("ERRORE NANOSLEEP: NAVE %d\n", getpid());
	} 
	position.x = ptr_posizioni[i].x;
	position.y = ptr_posizioni[i].y;
	printf("NAVE %d: giunta al porto piu' vicino.\n", indice);

	capacita = SO_CAPACITY;
	do {	
		if(sem_reserve(id_semaforo_gestione, 0) == -1){
			ERROR("nella NAVE causato dal sem_reserve()")
			TEST_ERROR
		}
		if(sem_waitforzero(id_semaforo_gestione, 0) == -1){
			ERROR("nella NAVE causato dal sem_waitforzero()")
			TEST_ERROR
		}
		pause();
	} while(DATA < SO_DAYS+1);
	exit(199);
}

void sigusr1_handler(int signum){
	printf("NAVE %d: RICEVUTO SIGUSR1\n", getpid());
	DATA++;
}

void sigusr2_handler(int signum){
	/* sgancio e uscita*/
	/*printf("nave 1:\n");*/
	sgancia_shm(ptr_lotti);
	/*printf("nave 2:\n");*/
	sgancia_shm(ptr_mercato);
	/*printf("nave 3:\n");*/
	sgancia_shm(ptr_posizioni);
	/*printf("nave 4:\n");*/
	sgancia_shm(ptr_dump);
	/*printf("nave 5:\n");*/
	/*printf("NAVE %d:", getpid());*/
	exit(10+indice);
}