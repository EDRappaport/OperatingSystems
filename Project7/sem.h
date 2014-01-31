/* sem.h

Rappaport, Elliot D
ECE357: Operating Systems
November 20, 2013
*/

#ifndef SEM_H_INCLUDED
#define SEM_H_INCLUDED

#define MAX_SUPPORTED_PROCS 64

struct sem{
	char spinlock;
	unsigned int sem_count;
	int waitingPIDs[MAX_SUPPORTED_PROCS];
	int nextNewWaiter, nextReadWaiter; int numWaiters;
};

int tas (volatile char *lock);

void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s);
void sem_inc(struct sem *s);

#endif