/* indice dei parametri */
#define SO_NAVI 0
#define SO_PORTI 1
#define SO_MERCI 2
#define SO_SIZE 3
#define SO_MIN_VITA 4
#define SO_MAX_VITA 5
#define SO_LATO 6
#define SO_SPEED 7
#define SO_CAPACITY 8
#define SO_BANCHINE 9
#define SO_FILL 10
#define SO_LOADSPEED 11
#define SO_DAYS 12

/* Quanti byte può occupare una riga di parametri del file di configurazione. */
#define MAX_FILE_STR_LEN 60

/* Indica quanti parametri vanno inseriti a tempo di esecuzione. */
#define QNT_PARAMETRI 13

/* i byte necessari per il dump (shm) */
#define SIZE_DUMP 100

/* key del dump (shm) */
#define DUMP_KEY 11

/* n. di byte usati da ogni porto nella shm "registro porti" */
#define PORTO_REGISTER_SIZE 100

/* i byte necessari per il registro dei porti (shm) */
#define SIZE_MEM_PORTI (PARAMETRO[SO_PORTI]*PORTO_REGISTER_SIZE)

/* key del registro dei porti (shm) */
#define PORTI_MEM_KEY 12

/* key del set di semafori per gestire le banchine*/
#define BANCHINE_SEM_KEY 21

typedef struct _position{
	double x;
	double y;
} point;