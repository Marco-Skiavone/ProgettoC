#include "queue_lib.h"

int set_coda_richieste(key_t key) {
    int coda_id;
    if ((coda_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
        perror("msgget creazione coda");
        exit(255);
    }
    return coda_id;
}

int get_coda_id(key_t key) {
    int coda_id;
    if ((coda_id = msgget(key, 0666)) == -1) {
        perror("msgget coda");
        exit(255);
    }
    return coda_id;
}

void distruggi_coda(int coda_id) {
    if (msgctl(coda_id, IPC_RMID, NULL) == -1) {
        perror("msgctl destroy coda");
        exit(255);
    }
}

void invia_richiesta(richiesta r, int coda_id, int fd_fifo){
    r.mtype += 1;
    /* inserito IPC_NOWAIT per le troppe richieste che eventualmente possono bloccare la simulazione. */
    if(msgsnd(coda_id, &r, MSG_SIZE, IPC_NOWAIT) == -1){    
        if(errno == EAGAIN){
            /* passa richiesta su fifo! */
            write(fd_fifo, &r, sizeof(richiesta));
            errno = 0;
        }
        TEST_ERROR
    }
}

richiesta accetta_richiesta(int msgtype, int coda_id){
    richiesta r;
    if(msgrcv(coda_id, &r, MSG_SIZE, msgtype+1, IPC_NOWAIT) == -1){
        if(errno == 42){
            r.mtext.indicemerce = -1;
            
        }else{
            perror("accetta richiesta");
            exit(255);
        }
    }
    r.mtype -=1;
    return r;
}