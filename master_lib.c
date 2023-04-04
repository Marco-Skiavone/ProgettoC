#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "master_lib.h"

void clearLog(){
	fclose(fopen("out.txt", "w"));
	fclose(fopen("log_dump.txt", "w"));
	fclose(fopen("log_mercato.txt", "w"));
	fclose(fopen("log_navi.txt","w"));
}


void alloca_id(int *id_shm_mercato, int *id_shm_dettagli_lotti, int *id_shm_posizioni_porti, int *id_shm_dump, int *id_coda_richieste, int PARAMETRO[]){
	*(id_shm_mercato) = alloca_shm(CHIAVE_SHAREDM_MERCATO, SIZE_SHAREDM_MERCATO);
	*(id_shm_dettagli_lotti) = alloca_shm(CHIAVE_SHAREDM_DETTAGLI_LOTTI, SIZE_SHAREDM_DETTAGLI_LOTTI);
	*(id_shm_posizioni_porti) = alloca_shm(CHIAVE_SHAREDM_POSIZIONI_PORTI, SIZE_SHAREDM_POSIZIONI_PORTI);
	*(id_shm_dump) = alloca_shm(CHIAVE_SHAREDM_DUMP, SIZE_SHAREDM_DUMP);
	*(id_coda_richieste) = set_coda_richieste(CHIAVE_CODA);
	printf("SHARED_MEM_MERCATO: %d\n", *(id_shm_mercato));
	printf("SHARED_MEM_DETTAGLI_LOTTI: %d\n", *(id_shm_dettagli_lotti));
	printf("SHARED_MEM_POSIZIONI_PORTI: %d\n", *(id_shm_posizioni_porti));
	printf("SHARED_MEM_DUMP: %d\n", *(id_shm_dump));
	printf("CODA RICHIESTE: %d\n", *(id_coda_richieste));
    printf("__________________________ \n\n");
}

point generate_random_point_master(int lato) {
    int mant, p_intera;
    point p;
    p_intera = mant = rand()%lato;

	p.x = ((double)mant/lato) + (p_intera*getppid()%lato);
	p_intera = mant = rand()%lato;
	p.y = ((double)mant/lato) + ((p_intera*getppid()%lato));
    return p;
}

void generate_positions(double lato, point* posizioni_porti, int PORTI) {
    int i, j, found;
    point p;
    posizioni_porti[0].x = 0;       posizioni_porti[0].y = 0;
    posizioni_porti[1].x = lato;    posizioni_porti[1].y = 0;
    posizioni_porti[2].x = lato;    posizioni_porti[2].y = lato;
    posizioni_porti[3].x = 0;       posizioni_porti[3].y = lato;

    for (i = 4; i < PORTI; i++) {
        p = generate_random_point_master(lato);
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


void inizializza_dump(void *vptr_shm_dump, int PARAMETRO[]){
    int i;
    CAST_DUMP(vptr_shm_dump)->data = 0;
    CAST_DUMP(vptr_shm_dump)->merce_dump_ptr = (merce_dump*)(vptr_shm_dump+sizeof(int));
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
}

void setUpLotto(merce* ptr_dettagli_lotti, int PARAMETRO[]){
    int i;
    ptr_dettagli_lotti[0].val = 1;
    ptr_dettagli_lotti[0].exp = SO_MIN_VITA + (rand() % (SO_MAX_VITA - SO_MIN_VITA));
    for(i=1;i<SO_MERCI;i++){
        ptr_dettagli_lotti[i].val = (rand() & SO_SIZE) + 1;
        ptr_dettagli_lotti[i].exp = SO_MIN_VITA + (rand() % (SO_MAX_VITA - SO_MIN_VITA));
    }
}

int equals(double x, double y){
	if(x > y)
		return (x-y) < TOLLERANZA ? 1: 0;
	else 
		return (y-x) < TOLLERANZA ? 1 : 0;
}

void stampa_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indice_porto){
	int j, totale;
	totale = 0;
	printf("MERCATO, giorno %d\n", CAST_DUMP(vptr_shm_dump)->data);
	printf("PORTO %d:\n", indice_porto);
	for(j = 0; j < SO_MERCI; j++){
		printf("[%d, %d] ", j, (CAST_MERCATO(vptr_shm_mercato))[indice_porto][j].val);
		if((CAST_MERCATO(vptr_shm_mercato))[indice_porto][j].val>0){
			totale += (CAST_MERCATO(vptr_shm_mercato))[indice_porto][j].val;
		}
	}
	printf(" presente: %d", totale);
	printf("\n");
	
}

int controlla_mercato(void *vptr_shm_mercato, void *vptr_shm_dump, int PARAMETRO[]){
	int i, j;
	int offerte, richieste;
	offerte = richieste = 0;
	for(i = 0; i < SO_PORTI && (!richieste || !offerte); i++){
		for(j = 0; j < SO_MERCI && (!richieste || !offerte); j++){
			if(!offerte && CAST_MERCATO(vptr_shm_mercato)[i][j].val > 0 && CAST_MERCATO(vptr_shm_mercato)[i][j].exp > CAST_DUMP(vptr_shm_dump)->data){
				offerte = 1;
			}
			if(!richieste && CAST_MERCATO(vptr_shm_mercato)[i][j].val < 0){
				richieste = 1;
			}
		}
	}
	return offerte || richieste;
}

void stampa_merci_porti_navi(int PARAMETRO[], void * vptr_shm_dump, void *vptr_shm_mercato, int id_semaforo_banchine){
	int i, j;
	j=0;
	/*da togliere in futuro --> vptr_shm_mercato e stampa_mercato_dump()*/
	for(i = 0; i < (SO_MERCI+SO_PORTI); i++){
		
        if(i < SO_MERCI){  /* stampo merci per tipologia */
            printf("Merce %d\n", i);
            printf("- consegnata: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].consegnata);
            printf("- presente in nave: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].presente_in_nave);
            printf("- presente in porto: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].presente_in_porto);
            printf("- scaduta in nave: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].scaduta_in_nave);
            printf("- scaduta in porto: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].scaduta_in_porto);
        } else {
            printf("Porto %d\n", j);
            printf("- merce presente: %d\n",  (CAST_PORTO_DUMP(vptr_shm_dump))[j].mercepresente);
            printf("- merce ricevuta: %d\n", (CAST_PORTO_DUMP(vptr_shm_dump))[j].mercericevuta);
            printf("- merce spedita: %d\n", (CAST_PORTO_DUMP(vptr_shm_dump))[j].mercespedita);
            (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchineoccupate = (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchinetotali - sem_get_val(id_semaforo_banchine, j);
            printf("- banchine occupate/totali: %d/%d\n", (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchineoccupate, (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchinetotali);

			if(freopen("log_mercato.txt","a", stdout)==NULL)
        		{perror("freopen ha ritornato NULL");}
			stampa_mercato_dump(vptr_shm_dump, vptr_shm_mercato, PARAMETRO, j);
			if(freopen("log_dump.txt", "a", stdout)==NULL)
        		{perror("freopen ha ritornato NULL");}
            j++;
        }
    }
    printf("Navi:\n");
    printf("- navi in mare con carico: %d\n", CAST_DUMP(vptr_shm_dump)->nd.navicariche);
    printf("- navi in mare senza carico: %d\n", CAST_DUMP(vptr_shm_dump)->nd.naviscariche);
    printf("- navi in porto (carico/scarico): %d\n", CAST_DUMP(vptr_shm_dump)->nd.naviporto);
}

void stampa_dump(int PARAMETRO[], void * vptr_shm_dump, void *vptr_shm_mercato, int id_semaforo_banchine){
	if(freopen("log_dump.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
	/*da togliere in futuro --> vptr_shm_mercato e stampa_mercato_dump()*/
    printf("*** Inizio stampa del dump: giorno %d ***\n", ((dump*)vptr_shm_dump)->data);
	stampa_merci_porti_navi(PARAMETRO, vptr_shm_dump,vptr_shm_mercato, id_semaforo_banchine);
    printf("\n--- Fine stato dump attuale (giorno %d). ---\n", CAST_DUMP(vptr_shm_dump)->data);
	if(freopen("out.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
}

void calcola_porti_term(int PARAMETRO[], void* vptr_shm_dump){
	int i, j; 
	/* j sarà inizialmente uguale a 0, primo porto ==> primo confronto */
	for(i = 1; i < SO_PORTI; i++){
		j = CAST_TERM_DUMP(vptr_shm_dump).porto_spedite;
		if(CAST_PORTO_DUMP(vptr_shm_dump)[i].mercespedita > CAST_PORTO_DUMP(vptr_shm_dump)[j].mercespedita){
			CAST_TERM_DUMP(vptr_shm_dump).porto_spedite = i;
		}
		j = CAST_TERM_DUMP(vptr_shm_dump).porto_ricevute;
		if(CAST_PORTO_DUMP(vptr_shm_dump)[i].mercericevuta > CAST_PORTO_DUMP(vptr_shm_dump)[j].mercericevuta){
			CAST_TERM_DUMP(vptr_shm_dump).porto_ricevute = i;
		}
	}
}

void stampa_terminazione(int PARAMETRO[], void * vptr_shm_dump, void * vptr_shm_mercato, int id_semaforo_banchine){
	int i, j;
	/*da togliere in futuro --> vptr_shm_mercato e stampa_mercato_dump()*/
	if(freopen("log_dump.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
	printf("\n----------------------------------\n");
	printf(" *** STAMPA DI TERMINAZIONE DELLA SIMULAZIONE! giorno %d ***\n", CAST_DUMP(vptr_shm_dump)->data);
	stampa_merci_porti_navi(PARAMETRO, vptr_shm_dump,vptr_shm_mercato, id_semaforo_banchine);
	/* extra */
	printf("Report speciale di terminazione:\n");
	calcola_porti_term(PARAMETRO, vptr_shm_dump);
	printf("Porto che ha ricevuto più lotti di merce: %d\n", CAST_TERM_DUMP(vptr_shm_dump).porto_ricevute);
	printf("Porto che ha spedito più lotti di merce: %d\n", CAST_TERM_DUMP(vptr_shm_dump).porto_spedite);
	printf("\n----------------------------\n");
	if(freopen("out.txt", "a", stdout)==NULL)
        {perror("freopen ha ritornato NULL");}
}

/* distruzione finale delle risorse 
 * ----------------------------------- */


void distruggi_risorse(int id_mercato, int id_lotti, int id_posizioni, int id_dump, int id_coda){
    printf("DISTRUGGI_SHARED_MEM_MERCATO\n"); distruggi_shm(id_mercato);
    printf("DISTRUGGI_SHARED_MEM_DETTAGLI_LOTTI\n"); distruggi_shm(id_lotti);
    printf("DISTRUGGI_SHARED_MEM_POSIZIONI_PORTI\n"); distruggi_shm(id_posizioni);
    printf("DISTRUGGI_SHARED_MEM_DUMP\n"); distruggi_shm(id_dump);
    printf("DISTRUGGI_CODA_RICHIESTE\n"); distruggi_coda(id_coda);
	printf("__________________________ \n\n");
}

void distruggi_semafori(int id_sem_mercato, int id_sem_dump, int id_sem_banchine, int id_sem_gestione){
    printf("DISTRUGGI_SEM_MERCATO\n");  sem_destroy(id_sem_mercato);
    printf("DISTRUGGI_SEM_DUMP\n");     sem_destroy(id_sem_dump);
    printf("DISTRUGGI_SEM_BANCHINE\n"); sem_destroy(id_sem_banchine);
    printf("DISTRUGGI_SEM_GESTIONE\n"); sem_destroy(id_sem_gestione);
    printf("__________________________ \n");
}

void free_ptr(int *childs, char **argv_figli, int size){
	if(childs != NULL){
		free(childs);
	} else 
		perror("childs pointer is NULL");

	if(argv_figli != NULL){
		int i;
		for(i = 0; i < size; i++){
			if(argv_figli[i] != NULL)
			free(argv_figli[i]);
		}
		free(argv_figli);
	} else
		perror("argv_figli pointer is NULL");
}