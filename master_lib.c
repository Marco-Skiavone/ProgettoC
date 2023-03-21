#ifndef _DEFINITIONS_H
	#include "definitions.h"
#endif
#include "master_lib.h"

int equals(double x, double y){
	if(x > y)
		return (x-y) < TOLLERANZA ? 1: 0;
	else 
		return (y-x) < TOLLERANZA ? 1 : 0;
}

void generate_positions(double lato, point* posizioni_porti, int PORTI) {
    int i, j, found;
    point p;
    posizioni_porti[0].x = 0;       posizioni_porti[0].y = 0;
    posizioni_porti[1].x = lato;    posizioni_porti[1].y = 0;
    posizioni_porti[2].x = lato;    posizioni_porti[2].y = lato;
    posizioni_porti[3].x = 0;       posizioni_porti[3].y = lato;

    for (i = 4; i < PORTI; i++) {
        p = generate_random_point(lato);
        found = 0;
        for (j = 0; j < i && !found; j++) {
			if (equals(p.x, posizioni_porti[j].x) && (equals(p.y, posizioni_porti[j].y))) {
				found = 1;
			}
		}
		if (found) {
			i--;
		} else {
			posizioni_porti[i].x = p.x;
            posizioni_porti[i].y = p.y;
		}
    }
}

point generate_random_point(int lato) {
    int mant, p_intera;
    point p;
    p_intera = mant = rand()%lato;

	p.x = ((double)mant/lato) + (p_intera*getppid()%lato);
	p_intera = mant = rand()%lato;
	p.y = ((double)mant/lato) + ((p_intera*getppid()%lato));
    return p;
}

void setUpLotto(merce* ptr_dettagli_lotti, int nmerci, int so_size, int so_min_vita, int so_max_vita){
    int i;
    ptr_dettagli_lotti[0].val = 1;
    ptr_dettagli_lotti[0].exp = so_min_vita + (rand() % (so_max_vita - so_min_vita));
    for(i=1;i<nmerci;i++){
        ptr_dettagli_lotti[i].val = (rand() & so_size) + 1;
        ptr_dettagli_lotti[i].exp = so_min_vita + (rand() % (so_max_vita - so_min_vita));
    }
}