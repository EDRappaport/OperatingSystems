/* fifoTest2.c

Rappaport, Elliot D
ECE357: Operating Systems
November 20, 2013
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include "fifo.h"
#include "sem.h"

#define NUM_PROCS 5
#define NUM_WRITES 50

int main(int argc, char const *argv[])
{
	struct fifo *myFifo;
	myFifo = mmap(NULL, sizeof (struct fifo), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	fifo_init(myFifo);

	int j, k, i;
	
	int pidReader = fork();
	switch(pidReader){
		case -1:
			perror("Fork!!"); exit(-1);
			break;
		case 0:
			for(j=0; j < NUM_PROCS*NUM_WRITES; j++){
				fprintf(stderr, "%lu\n", fifo_rd(myFifo));
			}
			_exit(0);
			break;
		default:
			break;
	}

	int pidWriter[NUM_PROCS];
	for (k=0; k < NUM_PROCS; k++){
		pidWriter[k] = fork();
		switch(pidWriter[k]){
			case -1:
				perror("Fork!!"); exit(-1);
				break;
			case 0:
				//fprintf(stderr, "in Child, PID: %d\n", pidWriter[k]);
				for (i=0; i < NUM_WRITES; i++){
					fifo_wr(myFifo, (k*100000000)+i);
				}
				_exit(0);
				break;
			default:
				//fprintf(stderr, "in Parent, PID: %d\n", pidWriter[k]);
				break;
		}
	}

	int stat1, stat2, kk;
	wait(&stat1);
	for (kk=0; kk<NUM_PROCS; kk++) wait(&stat2);

	return 0;
}