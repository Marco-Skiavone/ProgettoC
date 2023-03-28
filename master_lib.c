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
