#include "definitions.h"
#include "my_lib.h"
#include <time.h>
#include <math.h>

int PARAMETRO[QNT_PARAMETRI];

typedef struct{ /*struct ritornata da porto_piu_vicino*/
	int indice_porto; /*i del ciclo*/
	int nanosec_nano;
	/*restituisco anche le coordinate del porto dove si troverà la nave dopo*/
	double x;
	double y;
} viaggio;

void spostamento(viaggio v, point *p){
	/*deve fare la nanosleep*/
	/*cambiare la posizione della nave*/
	nanosleep(v.nanosec_nano, NULL);/*vedere il secondo parametro e maschera di tutti i segnali*/
	p->x = v.x;
	p->y = v.y;
}
/*funzione spostamento nave*/
double distanza (double x, double y){
	/*pitagora*/
	double prova = (x*x) + (y*y);
	return sqrt(prova);/*ritorna la distanza percorsa*/
}

viaggio porto_piu_vicino(){	/*DA TESTARE*/
	viaggio porto;
	int v = 0;/*posizione porto da ritornare*/
	int i = 0;/*var ciclo e indice porto*/
	int id_porti; /*id memoria posizioni dei porti*/
	point *p;/*puntatore per leggere "l'array" di struct point*/
	double d;/*var ausiliaria, distanza porto precedente e/o il primo all'inizio del ciclo*/
	double a, b; /*variabili ausiliarie, indicano la posizione del porto precedente e/o
	il primo all'inizio del ciclo*/
	
	/*aggangio alla memoria posizioni(in sola lettura)*/
	id_porti = shmget(KEY_POSIZIONI, SIZE_POSIZIONI, 0666);
	p = shmat(id_porti, NULL, SHM_RDONLY);
	/*trova il porto più vicino alla nave*/
	a = p->x;
	b = p->y;
	d = distanza(a, b);
	while (i < SO_PORTI){
		p = p + sizeof(point);/*va al punto successivo*/
		/*leggo a due a due elementi della memoria (x e y)*/
		/*sizeof(point)*PARAMETRO[SO_PORTI] --> SIZE_POSIZIONI*/
		/**/
		if (d - distanza(p->x, p->y) > 0){
			v = i; /*fino ad ora questo è il porto più vicino*/
			d = distanza(p->x, p->y);
			a = p->x; b = p->y;
		} else{}
		i++;
	}
	/*come faccio a sapere quali porti? shared memory con posizioni dei porti*/
		/*controllo while... pitagora alla fine*/
	shmdt(p);
	/*calcolo nanosecondi di sleep per il viaggio*/
	/*distanza/SO_SPEED*/
	/*costruisce la struct da ritornare*/
	porto.indice_porto = v;
	/*errore approssimabile (double)/(int) la mappa è molto grande*/
	porto.nanosec_nano = (distanza(a, b)/SO_SPEED)*1000000; /*km / km/s*/
	porto.x = a;
	porto.y = b;
	return porto;
}

int main(int argc, char *argv[]){
	viaggio v;
	point *p;/*puntatore alla posizione della nave*/
	int id; /*id memoria condivisa*/
	char * dump;/*per fare shmat()*/
	int i, index;
	point position;
	if(argc != (1+QNT_PARAMETRI)){
		ERROR("nel passaggio dei parametri alla nave")
		exit(EXIT_FAILURE);
	}
	index = atoi(argv[0]);
	TEST_ERROR
	for(i = 1; i < argc; i++){
		PARAMETRO[i] = atoi(argv[i]);
		TEST_ERROR
	}

	printf("NAVE %d: parametri letti.\n", getpid());
	srand(SEED);
	/*generazione punto casuale (random lib.c)*/
	/*agganciamento risorse
	- shmget()
	- shmat()
	nanosleep per il porto più vicino 
	*/
	id = shmget(KEY_DUMP, SIZE_DUMP, 0666);
	dump = shmat(id, NULL, 0);
	/*...*/
	/*nanosleep();*/

	spostamento(v, p);
	/*shmdt() a fine programma*/	
	exit(EXIT_SUCCESS);


}


