#include "definitions.h"


/** Invia il segnale di tempesta (SIGUSR2) a una nave.
  *	@returns l'indice della nave colpita dalla tempesta. */
int tempesta_nave(int *navi_rallentate, posizione_navi *vettore_pids_navi, int PARAMETRO[]);

/** Invia il segnale di mareggiata al porto (SIGINT) e alle navi(SIGUSR2).
  * @returns l'indice del porto (a)mareggiato.
*/
int mareggiata_porto(int *porti_pids, posizione_navi *vettore_pids_navi, int PARAMETRO[]);

/** Stampa gli effetti del meteo (parte dopo ogni stampa del dump). */
void stampa_meteo(int nave_tempesta, int porto_mareggiato);

/** Stampa finale degli effetti del meteo. */
void stampa_meteo_fine_simulazione(int navi_tempestate[], int porti_mareggiati[], int giorni_simulazione);