#include "demone.h"
#include "queue_lib.h"
#include "definitions.h"
int fd_fifo;
void signal_handler(int signo);

int main(int argc, char *argv[]){
	int id_coda;
	struct sigaction sa;
	richiesta r;
	sa.sa_flags = 0;
	sa.sa_handler = signal_handler;
	sigemptyset(&(sa.sa_mask));
	sigaction(SIGUSR2, &sa, NULL);
	id_coda = get_coda_id(CHIAVE_CODA);
	if((fd_fifo = open(NOME_FIFO, O_RDONLY)) == -1){
		fprintf("File %s, %d: Errore nell'apertura della FIFO!\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	do {
		if(read(fd_fifo, &r, sizeof(richiesta)) == -1){
			fprintf("File %s, %d: Errore nella lettura dalla FIFO!\n", __FILE__, __LINE__);
		}
		if(msgsnd(id_coda, &r, MSG_SIZE, 0) == -1){
			fprintf(stderr, "File %s, %d: Errore nella msgsnd da FIFO a coda!\n", __FILE__, __LINE__);
		}
	} while(1);
}

void signal_handler(int signo){
	switch (signo) {
		case SIGUSR2:
			close(fd_fifo);
			exit(EXIT_SUCCESS);
			break;
		default:
			fprintf(stderr, "File %s, %d: giunto segnale non contemplato!\n", __FILE__, __LINE__);
			break;
	}
}