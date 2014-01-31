/* fifo.c

Rappaport, Elliot D
ECE357: Operating Systems
November 20, 2013
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "fifo.h"
#include "sem.h"


void fifo_init(struct fifo *f){
	f->next_read = 0; f->next_write = 0;
	sem_init(&(f->sem_lock), 1);
	sem_init(&(f->sem_Read), 0);
	sem_init(&(f->sem_Write), MYFIFO_BUFSIZ);
}

void fifo_wr(struct fifo *f, unsigned long d){
	sem_wait(&(f->sem_Write));
	sem_wait(&(f->sem_lock));

	f->buf[f->next_write++] = d;
	f->next_write %= MYFIFO_BUFSIZ;
	
	sem_inc(&(f->sem_lock));
	sem_inc(&(f->sem_Read));
}

unsigned long fifo_rd(struct fifo *f){
	unsigned long d;
	
	sem_wait(&(f->sem_Read));
	sem_wait(&(f->sem_lock));

	d = f->buf[f->next_read++];
	f->next_read %= MYFIFO_BUFSIZ;

	sem_inc(&(f->sem_lock));
	sem_inc(&(f->sem_Write));

	return d;
}
