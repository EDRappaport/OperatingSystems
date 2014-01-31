/* sem.c

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
#include "sem.h"

static void handler(int sn){
	//fprintf(stderr, "SIGUSR1 - Handled.\n");
}

void sem_init(struct sem *s, int count){
	s->sem_count = count;
	s->nextNewWaiter = 0; s->nextReadWaiter = 0;
	s->numWaiters = 0;
	s->spinlock = 0;
}

int sem_try(struct sem *s){
	while (tas(&(s->spinlock)) != 0) ;
	if (s->sem_count > 0){
		s->sem_count --;
		s->spinlock = 0;
		return 1;
	}
	s->spinlock = 0;
	return 0;
}

void sem_wait(struct sem *s){
	for(;;){
		while (tas(&(s->spinlock)) != 0) ;
		if (s->sem_count > 0){
			s->sem_count --;
			s->spinlock = 0;
			break;
		}
		s->waitingPIDs[(s->nextNewWaiter)++] = (int) getpid();
		s->nextNewWaiter %= MAX_SUPPORTED_PROCS;
		++(s->numWaiters);
		sigset_t new_mask;
		sigfillset(&new_mask);
		sigprocmask(SIG_BLOCK, &new_mask, NULL);
		sigdelset(&new_mask,SIGUSR1);
		signal(SIGUSR1, handler);
		s->spinlock = 0;
		sigsuspend(&new_mask);
	}
}

void sem_inc(struct sem *s){
	while(tas(&(s->spinlock)) != 0) ;
	if (++s->sem_count > 0 && s->numWaiters > 0){
		--(s->numWaiters);
		kill(s->waitingPIDs[(s->nextReadWaiter)++], SIGUSR1);
		s->nextReadWaiter %= MAX_SUPPORTED_PROCS;
	}
	s->spinlock = 0;
}