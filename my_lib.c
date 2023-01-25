#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "my_lib.h"

/* genera e restituisce un punto in maniera randomica, dato SO_LATO */
point generate_rand_point(int LATO){
	int mant, p_intera;
	point p;
	/*coordinata x*/
	p_intera = mant = rand()%LATO;
	/*parte decimale + parte intera*/
	p.x = ((double)mant/LATO) + (p_intera*getppid()%LATO);
	/*coordinata y*/
	p_intera = mant = rand()%LATO;
	p.y = ((double)mant/LATO) + ((p_intera*getppid()%LATO));
	return p;
}

/* Metodo per vedere se 2 double sono accettabilmente distanti (TOLLERANZA) s*/
int equals(double x, double y){
	if(x > y)
		return (x-y) < TOLLERANZA ? 1: 0;
	else 
		return (y-x) < TOLLERANZA ? 1 : 0;
}

int fine_richieste(int id){
	if(msgrcv(id, NULL, SIZE_MSG, 0, IPC_NOWAIT) == -1)
		if(errno == ENOMSG)
			return 1; /*coda finita*/
	
	return 0;
}

int fine_offerte(merce *ptr_mercato, int PORTI, int MERCI, int data){
	int i, j, continua;
	merce (*ptr)[MERCI] = (merce (*)[MERCI])ptr_mercato;
	for(i = 0, continua = 1; i < PORTI && continua; i++){
		for(j = 0; j < PORTI && continua; j++){
			continua = (ptr[i][j].val > 0);
			/* ^^ c'è ancora qualche offerta ^^ */
				/*continua = (ptr_mercato[i][j].exp > data) ? 0 : 1; */	
		}
	}
	return continua;
}

void alarm_handler(int signal){
	if(signal == SIGALRM){
		printf("MASTER: Partenza dump...\n");
	}
}