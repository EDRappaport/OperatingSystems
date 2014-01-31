/* simpleProg.c

Rappaport, Elliot D
ECE357: Operating Systems
November 20, 2013
*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

void fcn(){
	;
}


int main(int argc, char const *argv[])
{
	int i, numRuns = 1000000000;
	struct timespec startTime, curTime, *tpEmpty, *tpFcn, *tpSyscall;

	clock_gettime(CLOCK_REALTIME, &startTime);
	for (i=0; i<numRuns; i++){
		;
	}
	clock_gettime(CLOCK_REALTIME, &curTime);
	double totEmptyTime = ((double)curTime.tv_sec - (double)startTime.tv_sec)*1000000000 + (((double)curTime.tv_nsec - (double)startTime.tv_nsec));

	clock_gettime(CLOCK_REALTIME, &startTime);
	for (i=0; i<1.0*numRuns; i++){
		;
		fcn();
	}
	clock_gettime(CLOCK_REALTIME, &curTime);
	double totFcnTime = ((double)curTime.tv_sec - (double)startTime.tv_sec)*1000000000 + (((double)curTime.tv_nsec - (double)startTime.tv_nsec));

	clock_gettime(CLOCK_REALTIME, &startTime);
	for (i=0; i<numRuns; i++){
		getuid();
	}
	clock_gettime(CLOCK_REALTIME, &curTime);
	double totSyscallTime = ((double)curTime.tv_sec - (double)startTime.tv_sec)*1000000000 + (((double)curTime.tv_nsec - (double)startTime.tv_nsec));

	double perIteration = totEmptyTime/numRuns;
	double perFcn = totFcnTime/numRuns - perIteration;
	double perSysCall = totSyscallTime/numRuns - perIteration;

	printf("Time Per Iteration:\t%lf\n", perIteration);
	printf("Time Per Function Call:\t%lf\n", perFcn);
	printf("Time Per System Call:\t%lf\n", perSysCall);

	return 0;
}