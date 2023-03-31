#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#ifndef _SEM_LIB_H
	#include "sem_lib.h"
#endif

/* Richiede una banchina al semaforo di indice 'indice_porto', effettuando 
 * una maschera dei segnali per evitare spiacevoli 'loop' dello scheduler.
 * 
 * BLOCCA SIGUSR1.
 * Una volta eseguita la semop(), un eventuale segnale pendente viene consegnato. */
void richiedi_banchina(int id_semaforo_banchine, int indice_porto);

/* Modifica lo stato delle navi nel dump in base all'argomento (int) passato. */
void stato_nave(int stato, int id_semaforo_dump, void *vptr_shm_dump, int indice);
