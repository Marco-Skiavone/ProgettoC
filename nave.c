#include "definitions.h"
#include "my_lib.h"
#include "sem_lib.h"
#include "nave_lib.h"
#include "dump_lib.h"
#include "merci_lib.h"

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
	double distanza, tempocarico;
	point position;
 	int pass, reqlette;
	int indicedestinazione, indiceportoattraccato;
	int lottiscartati;
	int noncaricare;
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
	TEST_ERROR;
	ptr_posizioni = aggancia_shm(id_posizioni);
	ptr_lotti = aggancia_shm(id_lotti);
	ptr_mercato = aggancia_shm(id_mercato);
	ptr_dump = aggancia_shm(id_dump);



	/*printf("nave el-8\n");*/
	srand(getpid());
	/* calcolo primo viaggio*/
	position = generate_rand_point(SO_LATO);
	if(sem_reserve(id_semaforo_gestione, 0) == -1){
		ERROR("nella NAVE causato dal sem_reserve()")
		TEST_ERROR
	}
	if(sem_waitforzero(id_semaforo_gestione, 0) == -1){
		ERROR("nella NAVE causato dal sem_waitforzero()")
		TEST_ERROR
	}

	i=0;
	tempocarico = 1;
	pass = 0;
	reqlette = 0;
	lottiscartati=0;
	noncaricare = 0;
	int skip = 0;
	indicedestinazione = calcola_porto_piu_vicino(position, ptr_posizioni, SO_PORTI, SO_LATO);
	TEST_ERROR
	distanza = calcola_distanza(position.x, position.y, ptr_posizioni[indicedestinazione].x, ptr_posizioni[indicedestinazione].y);
	TEST_ERROR

	attesa(distanza, SO_SPEED);

	capacita = SO_CAPACITY;
	j = 0;
	if(sem_reserve(id_semaforo_banchine, indicedestinazione)==-1){
		ERROR("FAIL 1 SEM RESERVE")
	}
	indiceportoattraccato = indicedestinazione;
	position.x = ptr_posizioni[indicedestinazione].x;
	position.y = ptr_posizioni[indicedestinazione].y;
	printf("NAVE %d: giunta al porto piu' vicino %d.\n", indice, indicedestinazione);
	while (1){

		/* USEREMO L'INT i PER TENERE CONTO DEL NUMERO DI MERCI CARICATE
			OVVERO L'INDICE DEL VETTORE carico */
		
		if(sem_reserve(id_semaforo_mercato, indiceportoattraccato)==-1){
			ERROR("FAIL RESERVE")
		}
		printf("sono attraccata al mercato\n");
		skip = 1;
		for(k=0;k<SO_MERCI && skip;k++){
			if(((merce (*)[SO_MERCI])ptr_mercato)[indiceportoattraccato][k].val>0){
				skip=0;
				printf("porto da NON skippare, no merci\n");
			}
		}


		do{
			printf("skip = %d\n", skip);
			if(skip){
				printf("Devo skippare il porto %d\n", indiceportoattraccato);
				indicedestinazione = calcola_porto_piu_vicino(position, ptr_posizioni, SO_PORTI, SO_LATO);
				printf("indice attracco: %d, indice destinazione (dopo): %d.\n", indiceportoattraccato, indicedestinazione);
				distanza = calcola_distanza(position.x, position.y, ptr_posizioni[indicedestinazione].x, ptr_posizioni[indicedestinazione].y);
				printf("distanza al prossimo porto: %g", distanza);
				capacita = SO_CAPACITY;
				tempocarico = 0;
				break;
			}

			/* ACCETTA LA PRIMA RICHIESTA DALLA CODA*/
			rkst = accettaRichiesta(-1);
			if(rkst.mtext.indicemerce == -1){
				ERROR("Errore accettazione prima richiesta.")
			}

			/* DA FARE UN BREAK I GUESS ?????? */
			distanza = calcola_distanza(position.x, position.y, ptr_posizioni[rkst.mtype].x, ptr_posizioni[rkst.mtype].y) / SO_SPEED;

			/* CONTROLLA QUANTI LOTTI DELLA RICHIESTA PUOI CARICARE */
			while(rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val < capacita){
        		rkst.mtext.nlotti--;
				lottiscartati++;
    		}

			/* CHIEDI AL PORTO SE HA DISPONIBILITA PER QUELLE RICHIESTE */

			while(rkst.mtext.nlotti >  ((merce (*)[SO_MERCI])ptr_mercato)[indiceportoattraccato][rkst.mtext.indicemerce].val){
				rkst.mtext.nlotti--;
				lottiscartati++;
			}

			/* AGGIORNO IL TEMPO DA PASSARE IL PORTO A CARICARE
				UN TEMPO PROVVISORIO IN INT E "ALLARGATO" CON UN +1 INIZIALE PER GARANTIRE
				UN MARGINE DI RITARDO NELLE OPERAZIONI DI CALCOLO E NANOSLEEP. IL TEMPO EFFETTIVO VERRA CALCOLATO PRIMA DELLA NANOSLEEP.
				QUESTO TEMPOCARICO VIENE RADDOPPIATO (*2) PER TENERE CONTO ANCHE DEL TEMPO CHE SI PASSERÀ
				A SCARICARE LE MERCI
				*/
			tempocarico += (((rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val) / SO_LOADSPEED)*2);


			/* CARICA DISPONIBILITA */
			/* distanza viene diviso per SO_SPEED e passato 
			 * come int approssimato per eccesso */
			if((ptr_lotti[rkst.mtext.indicemerce].exp > ( ((distanza/SO_SPEED)) + tempocarico + DATA)) && rkst.mtext.nlotti>0){

				capacita -= ptr_lotti[rkst.mtext.indicemerce].val * rkst.mtext.nlotti;

				carico[i].indice = rkst.mtext.indicemerce;
				carico[i].mer.val = rkst.mtext.nlotti;
				carico[i].mer.exp = ptr_lotti[rkst.mtext.indicemerce].exp;

				((merce (*)[SO_MERCI])ptr_mercato)[indiceportoattraccato][rkst.mtext.indicemerce].val -= rkst.mtext.nlotti;

				printf("ho caricato %d lotti di %d merce\n", carico[i].mer.val, carico[i].indice);
				i++;
				/* CONTROLLO SE NON SI È STATI IN GRADO DI SCARTARE TUTTO */
				if(lottiscartati!=0){
					rkst.mtext.nlotti = lottiscartati;
					inviaRichiesta(rkst);
				}
				indicedestinazione = rkst.mtype;
				pass=1;
				printf("la mia destinazione: %d\n", indicedestinazione);
			}else{
				/* NON STO CARICANDO LA DISPONIBILITÀ */
				tempocarico -= (((rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val) / SO_LOADSPEED)*2);
				rkst.mtext.nlotti += lottiscartati;
				inviaRichiesta(rkst);

				//printf("ho scartato la richiesta\n");
				lottiscartati = 0;
				pass=0;
				reqlette++;
			}
		}while(!pass && (reqlette < (SO_MERCI/2)));
		if(reqlette >= SO_MERCI/2){
			skip = 1;
		}

		lottiscartati = 0;
		pass = 0;
		// reqlette = 0;
		do{
			if(skip){
				break;
			}
			/* TROVA RICHIESTE PER IL PORTO DI DESTINAZIONE */
			rkst = accettaRichiesta(indicedestinazione);
			if(rkst.mtext.indicemerce = -1){
				pass = 1;
			}else{

				/* CONTROLLA QUANTI LOTTI DELLA RICHIESTA PUOI CARICARE */
				while(rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val < capacita){
    	    		rkst.mtext.nlotti--;
					lottiscartati++;
    			}

				/* CHIEDI AL PORTO SE HA DISPONIBILITA PER QUELLE RICHIESTE */

				while(rkst.mtext.nlotti >  ((merce (*)[SO_MERCI])ptr_mercato)[indiceportoattraccato][rkst.mtext.indicemerce].val){
					rkst.mtext.nlotti--;
					lottiscartati++;
				}

				/* TEMPO CARICO PROVVISORIO */
				tempocarico += (int) (((rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val) / SO_LOADSPEED) * 2);
				capacita -= ptr_lotti[rkst.mtext.indicemerce].val * rkst.mtext.nlotti;

				if(ptr_lotti[rkst.mtext.indicemerce].exp < (tempocarico + distanza + DATA)){
					capacita += ptr_lotti[rkst.mtext.indicemerce].val * rkst.mtext.nlotti;
					tempocarico -= (((rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val) / SO_LOADSPEED) * 2);
					rkst.mtext.nlotti += lottiscartati;
					inviaRichiesta(rkst);
				}else{

					for(j=0;j<i;j++){
						if(ptr_lotti[carico[j].indice].exp < (distanza + tempocarico + DATA) ){
							noncaricare = 1;
						}
					}
					if(noncaricare){
						tempocarico -= (((rkst.mtext.nlotti * ptr_lotti[rkst.mtext.indicemerce].val) / SO_LOADSPEED) * 2);
						capacita += ptr_lotti[rkst.mtext.indicemerce].val * rkst.mtext.nlotti;
						rkst.mtext.nlotti += lottiscartati;
						inviaRichiesta(rkst);
					}else{
						carico[i].indice = rkst.mtext.indicemerce;
						carico[i].mer.val = rkst.mtext.nlotti;
						carico[i].mer.exp = ptr_lotti[rkst.mtext.indicemerce].exp;

						((merce (*)[SO_MERCI])ptr_mercato)[indiceportoattraccato][rkst.mtext.indicemerce].val -= rkst.mtext.nlotti;

						i++;
					}

				}

			}
			reqlette++;
		}while(!pass && reqlette<(SO_MERCI/2) && i<MAX_RICHIESTE);
		if(reqlette>= SO_MERCI/2){
			printf("Devo skippare il porto %d per reqlette\n", indiceportoattraccato);
			indicedestinazione = calcola_porto_piu_vicino(position, ptr_posizioni, SO_PORTI, SO_LATO);
			printf("indice attracco: %d, indice destinazione (dopo): %d.\n", indiceportoattraccato, indicedestinazione);
			distanza = calcola_distanza(position.x, position.y, ptr_posizioni[indicedestinazione].x, ptr_posizioni[indicedestinazione].y);
			printf("distanza al prossimo porto: %g", distanza);
			capacita = SO_CAPACITY;
			tempocarico = 0;
			break;
		}
		sem_release(id_semaforo_mercato, indiceportoattraccato);
		TEST_ERROR
		/* CALCOLO TEMPO CARICO EFFETTIVO */
		/* capacita/SO_LOADSPEED */
		/* NANOSLEEP PER CARICARE */
		attesa((SO_CAPACITY-capacita), SO_LOADSPEED);
		/* LIBERA IL SEMAFORO */
		sem_release(id_semaforo_banchine, indiceportoattraccato);
		TEST_ERROR
		/* AGGIORNA IL DUMP*/

		/* NANOSLEEP PER ANDARE AL PORTO DI DESTINAZIONE */
		attesa(distanza, SO_SPEED);
		TEST_ERROR
		/* CHIEDI IL SEMAFORO PER LE BANCHINE */
		position.x = ptr_posizioni[indicedestinazione].x;
		position.y = ptr_posizioni[indicedestinazione].y;
		if(sem_reserve(id_semaforo_banchine, indicedestinazione) == -1){
			ERROR("FAIL SEM RESERVE")
		}
		indiceportoattraccato = indicedestinazione;
		printf("sono attraccata alla banchina porto %d\n", indiceportoattraccato);
		/*AGGIORNARE DUMP */

		/* CODICE PER SCARICARE LA MERCE A DESTINAZIONE */
		attesa((SO_CAPACITY-capacita), SO_LOADSPEED);
		/*data_scarico = DATA;*/
		if(sem_reserve(id_semaforo_mercato, indicedestinazione) != -1){
			/* uso data_scarico per vedere la scadenza */
			for(j=0;j<i;i++){
				scaricamerci(carico[i].mer, indicedestinazione, carico[i].indice, DATA, SO_MERCI, ptr_mercato);
			}
			sem_release(id_semaforo_mercato, indicedestinazione);
		}

		capacita = SO_CAPACITY;
		tempocarico = 1;
		lottiscartati = 0;
		noncaricare = 0;
		i=0;
	}


	sleep(10);

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