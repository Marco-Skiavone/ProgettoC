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