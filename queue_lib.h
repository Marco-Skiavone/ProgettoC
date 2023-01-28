#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif

int set_coda_richieste(key_t key);

int get_coda_id(key_t key);

void destroy_coda(int coda_id);

void invia_richiesta(richiesta r, int coda_id);

richiesta accetta_richiesta(int msgtype, int coda_id);