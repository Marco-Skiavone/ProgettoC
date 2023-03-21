#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

/* Ritorna 1 se i due double dati sono ragionevolmente uguali, 0 altrimenti.
 * Fa uso di TOLLERANZA. */
int equals(double x, double y);

/* Genera le posizioni dei porti. */
void generate_positions(double lato, point* posizioni_porti, int PORTI);

/* Genera e restituisce un 'point' generico. */
point generate_random_point(int lato);

/* Genera randomicamente i lotti (ton/lotto e scadenza) per ogni tipo di merce.
 * 
 * NB: ogni tipo di merce avrà un proprio numero, 
 * fisso per la simulazione corrente, di grandezza dei lotti. Ci sarà sempre un lotto da 1 ton/lotto
 * al fine di evitare problemi nella generazioni dei lotti su SO_FILL/SO_PORTI. */
void setUpLotto(merce* ptr_dettagli_lotti, int nmerci, int so_size, int so_min_vita, int so_max_vita);