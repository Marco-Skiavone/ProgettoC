#include "meteo_lib.h"

int tempesta_nave(posizione_navi *vettore_pids_navi, int PARAMETRO[]){
    int i, nave_tempesta = -1;
    for(i=0;i<SO_NAVI;i++){
        if(vettore_pids_navi[i].indice_porto == -1){
            kill(vettore_pids_navi[i].pid, SIGUSR2);
            nave_tempesta = i;
            break;
        }
    }
	return nave_tempesta;
}

int mareggiata_porto(int *porti_pids, posizione_navi *vettore_pids_navi, int PARAMETRO[]){
    int i, porto_mareggiato;
    porto_mareggiato = rand() % SO_PORTI + 0;
    kill(porti_pids[porto_mareggiato], SIGINT);
    for(i=0;i<SO_NAVI;i++){
        if(vettore_pids_navi[i].indice_porto == porto_mareggiato){
            kill(vettore_pids_navi[i].pid, SIGINT);
        }
    }
    return porto_mareggiato;
}

void stampa_meteo(int nave_tempesta, int porto_mareggiato){
    
    if(nave_tempesta != -1){
        fprintf(stdout, "Nave %d ha incrociato una tempesta\n", nave_tempesta);
    }
    fprintf(stdout, "Mareggiata in porto %d", porto_mareggiato);

}

void stampa_meteo_fine_simulazione(int navi_tempestate[], int porti_mareggiati[], int giorni_simulazione){
    int i;
    fprintf(stdout, "Resoconto meteo:\n");
    for(i=0;i<giorni_simulazione;i++){
        fprintf(stdout, "Giorno %d\n", i);
        if(navi_tempestate[i]!=-1){
            fprintf(stdout, "Nave %d in tempesta\n", navi_tempestate[i]);
        }
        fprintf(stdout, "Porto %d interessato da una mareggiata\n", porti_mareggiati[i]);
    }
}