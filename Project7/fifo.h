/* fifo.h

Rappaport, Elliot D
ECE357: Operating Systems
November 20, 2013
*/

#ifndef FIFO_H_INCLUDED
#define FIFO_H_INCLUDED

#include "sem.h"

#define MYFIFO_BUFSIZ 4096
struct fifo{
	unsigned long buf[MYFIFO_BUFSIZ];
	int next_write, next_read;
	int item_count;
	struct sem sem_lock, sem_Read, sem_Write;
};

void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f, unsigned long d);
unsigned long fifo_rd(struct fifo *f);

#endif
