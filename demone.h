#ifndef _DEMONE_H
#define _DEMONE_H
#define _GNU_SOURCE
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

/* Tipo del corpo del messaggio. */
typedef struct {
	int indicemerce;
    int nlotti;
} m_text;

/* Tipo base di richiesta da inserire in CODA MSG*/
typedef struct {
    long mtype;
    m_text mtext;
} richiesta;

/* Size in byte dei messaggi */
#define MSG_SIZE (sizeof(m_text))
/* Chiave della coda richieste */
#define CHIAVE_CODA 50
/* Pathname della fifo scelta */
#define NOME_FIFO "fifo_coda_messaggi"

#endif