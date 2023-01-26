#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "dump_lib.h"

/* funzione interna per calcolare lo spazio necessario per il dump */
int calcola_spazio_necessario(int par_SO_MERCI, int par_SO_PORTI){
    int spazio;
    spazio = sizeof( porto_dump) * par_SO_PORTI;
    spazio += sizeof( merce_dump) * par_SO_MERCI;
    spazio += sizeof( dump);
    return spazio;
}

/* Crea il dump e ne ritorna il puntatore */
int alloca_shm_dump(int par_SO_MERCI, int par_SO_PORTI){
    int spazio = calcola_spazio_necessario(par_SO_MERCI, par_SO_PORTI);
    int id_dump = shmget(KEY_DUMP, spazio, IPC_CREAT | IPC_EXCL | PERMESSI);
    TEST_ERROR
    return id_dump;
}


/* fa la get del dump senza crearlo */
int set_shm_dump(int MERCI, int PORTI){
    int id_dump = shmget(KEY_DUMP, calcola_spazio_necessario(MERCI, PORTI), PERMESSI);
    TEST_ERROR
    return id_dump;
}

void stampa_dump(dump *ptr_dump, int MERCI, int PORTI, int data){
    int i, j;
    printf("*** Inizio stampa del dump: giorno %d ***\n", data);
    for(i = 0; i < MERCI+PORTI; i++){
        if(i < MERCI){  /* stampo merci per tipologia */
            printf("Merce %d\n", i);
            printf("- consegnata: %d\n", ptr_dump->merce_dump_ptr[i].consegnata);
            TEST_ERROR
            printf("- presente in nave: %d\n", ptr_dump->merce_dump_ptr[i].presente_in_nave);
            printf("- presente in porto: %d\n", ptr_dump->merce_dump_ptr[i].presente_in_porto);
            printf("- scaduta in nave: %d\n", ptr_dump->merce_dump_ptr[i].scaduta_in_nave);
            printf("- scaduta in porto: %d\n", ptr_dump->merce_dump_ptr[i].scaduta_in_porto);
        } else if(i < PORTI) {
            j = i - MERCI;
            printf("Porto %d\n", i);
            printf("- merce presente: %d\n", ptr_dump->porto_dump_ptr[j].mercepresente);
            TEST_ERROR
            printf("- merce ricevuta: %d\n", ptr_dump->porto_dump_ptr[j].mercericevuta);
            printf("- merce spedita: %d\n", ptr_dump->porto_dump_ptr[j].mercespedita);
            printf("- banchine occupate/totali: %d/%d\n", ptr_dump->porto_dump_ptr[j].banchineoccupate, ptr_dump->porto_dump_ptr->banchinetotali);
        }
    }
    printf("Navi:\n");
    printf("- navi in mare con carico: %d\n", ptr_dump->nd.navicariche);
    printf("- navi in mare senza carico: %d\n", ptr_dump->nd.naviscariche);
    printf("- navi in porto (carico/scarico): %d\n", ptr_dump->nd.naviporto);
    printf("\n--- Fine stato dump attuale (giorno %d). ---\n", data);
}