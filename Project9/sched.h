#ifndef SCHED_DEFINED
#define SCHED_DEFINED

#include "savectx.h"

#define SCHED_NPROC 256

#define SCHED_READY 0
#define SCHED_RUNNING 1
#define SCHED_SLEEPING 2
#define SCHED_ZOMBIE 3

struct sched_proc{
	int taskState, priority, vruntime, timeSlice;
	int retVal;
	int nice ;
	int pid, ppid;
	char *stackPtr;
	void *sp;
	struct savectx ctx;

	char *Name;

	struct sched_proc *next;
	struct sched_proc *prev;
};

struct sched_waitq{
	struct sched_proc *next;
	struct sched_proc *prev;
};

#endif