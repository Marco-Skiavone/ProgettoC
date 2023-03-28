#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "master_lib.h"

int confronto_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indice_porto){
	int i, j, presente, trovata;
	presente = trovata = 0;
	for(i = 0; i < SO_MERCI; i++){
		if(CAST_MERCATO(vptr_shm_mercato)[i][j].val > 0){
			presente += CAST_MERCATO(vptr_shm_mercato)[indice_porto][i].val;
		}
	}
	trovata += CAST_PORTO_DUMP(vptr_shm_dump)[indice_porto].mercepresente;
	if(presente != trovata){
		printf("\nTROVATO ERRORE NEL DUMP AL PORTO %d: presente=%d, trovata=%d!\n", indice_porto, presente, trovata);
		return 0;
	}
	return 1;
}

void stampa_mercato_dump(void *vptr_shm_dump, void *vptr_shm_mercato, int PARAMETRO[], int indice_porto){
	int j;
	printf("MERCATO, giorno %d\n", CAST_DUMP(vptr_shm_dump)->data);
	printf("PORTO %d:\n", indice_porto);
	for(j = 0; j < SO_MERCI; j++){
		printf("[%d, %d] ", j, (CAST_MERCATO(vptr_shm_mercato))[indice_porto][j].val);
	}
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

void stampa_dump(int PARAMETRO[], void * vptr_shm_dump, void *vptr_shm_mercato, int id_semaforo_banchine){
	/*da togliere in futuro --> vptr_shm_mercato e stampa_mercato_dump()*/
    int i, j;
    printf("*** Inizio stampa del dump: giorno %d ***\n", ((dump*)vptr_shm_dump)->data);
    for(i = 0; i < (SO_MERCI+SO_PORTI); i++){
        if(i < SO_MERCI){  /* stampo merci per tipologia */
            printf("Merce %d\n", i);
            printf("- consegnata: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].consegnata);
            TEST_ERROR
            printf("- presente in nave: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].presente_in_nave);
            printf("- presente in porto: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].presente_in_porto);
            printf("- scaduta in nave: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].scaduta_in_nave);
            printf("- scaduta in porto: %d\n", CAST_MERCE_DUMP(vptr_shm_dump)[i].scaduta_in_porto);
        } else {
            j = i - SO_MERCI;
            printf("Porto %d\n", j);
            printf("- merce presente: %d\n",  (CAST_PORTO_DUMP(vptr_shm_dump))[j].mercepresente);
            TEST_ERROR
            printf("- merce ricevuta: %d\n", (CAST_PORTO_DUMP(vptr_shm_dump))[j].mercericevuta);
            printf("- merce spedita: %d\n", (CAST_PORTO_DUMP(vptr_shm_dump))[j].mercespedita);
            (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchineoccupate = (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchinetotali - sem_get_val(id_semaforo_banchine, j);
            printf("- banchine occupate/totali: %d/%d\n", (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchineoccupate, (CAST_PORTO_DUMP(vptr_shm_dump))[j].banchinetotali);
            stampa_mercato_dump(vptr_shm_dump, vptr_shm_mercato, PARAMETRO, j);
        }
    }
    printf("Navi:\n");
    printf("- navi in mare con carico: %d\n", CAST_DUMP(vptr_shm_dump)->nd.navicariche);
    printf("- navi in mare senza carico: %d\n", CAST_DUMP(vptr_shm_dump)->nd.naviscariche);
    printf("- navi in porto (carico/scarico): %d\n", CAST_DUMP(vptr_shm_dump)->nd.naviporto);
    printf("\n--- Fine stato dump attuale (giorno %d). ---\n", CAST_DUMP(vptr_shm_dump)->data);
}