#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "nave_lib.h"

/* Esegue le nanosleep di spostamento, mascherando i segnali
 e settando lo stato sul dump, finita la nanosleep aggiorna la posizione */
void spostamento(viaggio v, point *p){
	struct timespec time;
	/* maschera segnale */
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	/* ---- */
	time.tv_sec = (__time_t)v.nanosec_nano/1E9;
	time.tv_nsec = ((__time_t)v.nanosec_nano) % time.tv_sec;
	nanosleep(&time, NULL);
	p->x = v.x;
	p->y = v.y;
	/* ripresa maschera precedente */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

/* ritorna il valore della nanosleep, 
 * "divisore" definisce il divisore per trovare i secondi */
int attesa(double to_wait, int divisore){
	int val_ritorno;
	struct timespec tempo;
	/* MASCHERA */
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	tempo.tv_sec = (long)(to_wait / divisore);
	tempo.tv_nsec = (long)to_wait % tempo.tv_sec; 
	if((tempo.tv_sec & 0) && (tempo.tv_nsec & 0))
		fprintf(stderr, "NAVE %d, linea %d: nanosleep chiamata con argomenti 0\n", getpid(), __LINE__);
	val_ritorno = nanosleep(&tempo, NULL);
	/* ripristino maschera */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	return val_ritorno;
}

double calcola_distanza(double x1, double y1, double x2, double y2 ){
	double a;
	double b;
	a = (x2-x1);
	b = (y2-y1);
	a = a*a;
	b = b*b;
	return sqrt(a+b);
}

int calcola_porto_piu_vicino(point p, point *ptr_shm_porti, int par_SO_PORTI){
	int i, indicemin;
	double distmin;
	indicemin = 0;
	distmin = calcola_distanza(p.x, p.y, ptr_shm_porti[0].x, ptr_shm_porti[0].y);
	for(i=0;i<par_SO_PORTI;i++){
		if(distmin>calcola_distanza(p.x,p.y, ptr_shm_porti[i].x, ptr_shm_porti[i].y)){
			indicemin = i;
			distmin = calcola_distanza(p.x,p.y, ptr_shm_porti[i].x, ptr_shm_porti[i].y);
		}
	}
	return indicemin;
}


/*restituisce un viaggio verso porto generico*/
viaggio porto_generico(int i_porto, point p_n, int SZ_POSIZIONI, int SPEED, point *ptr_posizioni){ 
	viaggio porto;
	int i = 0;
	int pp;
	while(i < i_porto){
		pp = pp + sizeof(point);
		i++;
	}
	porto.nanosec_nano = (calcola_distanza((ptr_posizioni+pp)->x, (ptr_posizioni+pp)->y, p_n.x, p_n.y)/SPEED)*1000000;
	return porto;
}

/*restituisce struct con info per il porto più vicino alla nave*/
viaggio porto_piu_vicino(point p_n, int SZ_POSIZIONI, int PORTI, int SPEED, point *ptr_posizioni){
	viaggio porto;
	int v = 0;
	int i = 0;
	int id_porti; 
	int pp;
	double a, b, d; 	
	a = p_n.x;
	b = p_n.y;
	d = calcola_distanza(a, b, p_n.x, p_n.y);
	/*scansiona i porti senza perdere il puntatore p = shmat()*/
	while (i < PORTI){
		pp = pp + sizeof(point);
		if (d - calcola_distanza((ptr_posizioni + pp)->x, (ptr_posizioni + pp)->y, p_n.x, p_n.y) > 0){
			v = i; 
			d = calcola_distanza((ptr_posizioni + pp)->x, (ptr_posizioni + pp)->y, p_n.x, p_n.y);
			a = (ptr_posizioni + pp)->x; b = (ptr_posizioni + pp)->y;
		}
		i++;
	}
	porto.indice_porto = v;
	porto.nanosec_nano = (calcola_distanza(a, b, p_n.x, p_n.y)/SPEED)*1000000;
	porto.x = a;
	porto.y = b;
	return porto;
}

/* se stato = 0: nave in porto */
void aggiorna_dump_nave(int indice_nave, int stato){
	int i;


}