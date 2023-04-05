#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "nave_lib.h"

point generate_random_point_nave(int lato) {
    srand(getpid());
    int mant, p_intera;
    point p;
    p_intera = mant = rand()%lato;

	p.x = ((double)mant/lato) + (p_intera*getppid()%lato);
	p_intera = mant = rand()%lato;
	p.y = ((double)mant/lato) + ((p_intera*getppid()%lato));
    return p;
}

double calcola_distanza(point p1, point p2){
    double a, b;
    a = (p1.x - p2.x);
    b = (p1.y - p2.y);
    a = a*a;
    b = b*b;
    return sqrt(a+b);
}

int calcola_porto_piu_vicino(point p, point* ptr_shm_posizioni_porti, int so_porti, int so_lato){
    int i, indicemin;
    double distmin, distanza;
    distmin = so_lato*so_lato;
    for(i=0;i<so_lato;i++){
        distanza = calcola_distanza(p, ptr_shm_posizioni_porti[i]);
        if(distanza < distmin){
            indicemin = i;
            distmin = distanza;
        }
    }
    return indicemin;
}

void richiedi_banchina(int id_semaforo_banchine, int indice_porto){
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    sem_reserve(id_semaforo_banchine, indice_porto);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void stato_nave(int stato, int id_semaforo_dump, void *vptr_shm_dump, int indice){
	switch(stato){
        case DN_MV_PORTO:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviscariche--;
            ((dump*)vptr_shm_dump)->nd.naviporto++;
            sem_release(id_semaforo_dump,1);
            printf("0. stato nave %d aggiornato\n", indice);
            break;
        case DN_MC_PORTO:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.navicariche--;
            ((dump*)vptr_shm_dump)->nd.naviporto++;
            sem_release(id_semaforo_dump,1);
            printf("1. stato nave %d aggiornato\n", indice);
            break;
        case DN_PORTO_MV:
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviporto--;
            ((dump*)vptr_shm_dump)->nd.naviscariche++;
            sem_release(id_semaforo_dump,1);
            printf("2. stato nave %d aggiornato\n", indice);
            break;
        case DN_PORTO_MC: 
            sem_reserve(id_semaforo_dump,1);
            ((dump*)vptr_shm_dump)->nd.naviporto--;
            ((dump*)vptr_shm_dump)->nd.navicariche++;
            sem_release(id_semaforo_dump,1);
            printf("3. stato nave %d aggiornato\n", indice);
            break;
        default:
            perror("**** ERRORE! Caso default di stato_nave()");
            break;
    }
}

void attesa(double val, int divisore) {
    struct timespec tempo;
    sigset_t mask, oldmask;
    int ret;
    double attesa_nanosleep;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    attesa_nanosleep = (val / divisore);	
    tempo.tv_sec = (__time_t)((attesa_nanosleep));
	tempo.tv_nsec = (__time_t)((attesa_nanosleep - ((__time_t)attesa_nanosleep))*1000000000); 

    ret = nanosleep(&tempo, NULL);
    if (ret != 0) {
        if (errno == EINTR){
            perror("nanosleep interrotta da un segnale");
        }else{
            perror("nanosleep fallita");
        }
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}

void aggiorna_dump_carico(void *vptr_dump, int indiceporto, merce_nave* carico, int caricati, int spazio_libero,  int id_sem_dump, int PARAMETRO[]){
    int i;
    //printf("Entro aggiorna dump sem reserve \n");
    sem_reserve(id_sem_dump, 0);
    if(spazio_libero != SO_CAPACITY){ 
        for(i = 0; i < caricati; i++){
            CAST_PORTO_DUMP(vptr_dump)[indiceporto].mercespedita += carico[i].mer.val;
            CAST_PORTO_DUMP(vptr_dump)[indiceporto].mercepresente -= carico[i].mer.val;
        
            CAST_MERCE_DUMP(vptr_dump)[carico[i].indice].presente_in_nave += carico[i].mer.val;
            CAST_MERCE_DUMP(vptr_dump)[carico[i].indice].presente_in_porto -= carico[i].mer.val;
        }
    }
    sem_release(id_sem_dump, 0);
    //printf("Esco aggiorna dump sem reserve \n");
}

void scaricamerci(merce scarico, int indiceporto, int indicemerce, int data, void* vptr_mercato, void* vptr_dump, int id_sem_dump, int PARAMETRO[]){    
    /* aggiorno mercato shm se possibile */
    if(scarico.exp >= data && CAST_MERCATO(vptr_mercato)[indiceporto][indicemerce].val <= -scarico.val){
        CAST_MERCATO(vptr_mercato)[indiceporto][indicemerce].val += scarico.val;
    } else if(CAST_MERCATO(vptr_mercato)[indiceporto][indicemerce].val > -scarico.val) {
        fprintf(stderr, "Errore in scarica_merci()\n");
    }
    sem_reserve(id_sem_dump, 0);
    if(scarico.exp >= data){ 
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].consegnata += scarico.val;
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].presente_in_nave -= scarico.val;
        CAST_PORTO_DUMP(vptr_dump)[indiceporto].mercericevuta += scarico.val;
    }else{
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].scaduta_in_nave += scarico.val;
        CAST_MERCE_DUMP(vptr_dump)[indicemerce].presente_in_nave -= scarico.val;
    }
    sem_release(id_sem_dump, 0);
}