#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "nave_lib.h"

/* Esegue le nanosleep di spostamento, mascherando i segnali
 e settando lo stato sul dump, finita la nanosleep aggiorna la posizione */
void spostamento(viaggio v, point *p){
	/* DA FINIRE */
	nanosleep(v.nanosec_nano, NULL);
	p->x = v.x;
	p->y = v.y;
}

/* Funzione spostamento nave*/
double distanza (double x, double y, point *p){
	return sqrt(((p->x - x)*(p->x - x))+((p->y - y)*(p->y -y)));
	/* alternativa a sqrt()
	double val, sqrt, temp;
	val = x*x + y*y;
	sqrt = val / 2;
	temp = 0;
	while(sqrt != temp){
		temp = sqrt;
		sqrt = ((val / temp) + temp) / 2;
	}
	return sqrt;
	*/
}

/*viaggio verso porto generico*/
viaggio porto_generico(int i_porto, point *p_n, int SZ_POSIZIONI, int SPEED){ 
	viaggio porto;
	int id_porti;
	point *p;
	int i = 0;
	int pp;
	id_porti = shmget(KEY_POSIZIONI, SZ_POSIZIONI, 0666);
	p = shmat(id_porti, NULL, SHM_RDONLY);
	while(i < i_porto){
		pp = pp + sizeof(point);
		i++;
	}
	porto.nanosec_nano = (distanza((p+pp)->x, (p+pp)->y, p_n)/SPEED)*1000000;

	return porto;
	shmdt(p);
}

/*restituisce struct con info per il porto più vicino alla nave*/
viaggio porto_piu_vicino(point *p_n, int SZ_POSIZIONI, int PORTI, int SPEED){
	viaggio porto;
	int v = 0;
	int i = 0;
	int id_porti; 
	point *p;
	int pp;
	double a, b, d; 	
	id_porti = shmget(KEY_POSIZIONI, SZ_POSIZIONI, 0666);
	p = shmat(id_porti, NULL, SHM_RDONLY);
	a = p_n->x;
	b = p_n->y;
	d = distanza(a, b, p_n);
	/*scansiona i porti senza perdere il puntatore p = shmat()*/
	while (i < PORTI){
		pp = pp + sizeof(point);
		if (d - distanza((p + pp)->x, (p + pp)->y, p_n) > 0){
			v = i; 
			d = distanza((p + pp)->x, (p + pp)->y, p_n);
			a = (p + pp)->x; b = (p + pp)->y;
		}
		i++;
	}
	shmdt(p);
	porto.indice_porto = v;
	porto.nanosec_nano = (distanza(a, b, p_n)/SPEED)*1000000;
	porto.x = a;
	porto.y = b;
	return porto;
}
