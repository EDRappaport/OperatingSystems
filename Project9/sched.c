#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include "sched.h"
#include "jmpbuf-offsets.h"

#define TIMER_MSECS 10
#define STACK_SIZE 64000


struct sched_proc *current;
struct sched_waitq waiting;

void sched_switch(){
	fprintf(stderr, "IN SWITCH\n");
	if (savectx(&(current->ctx)) == 0){

		//Add current to wait list:
		current->next = waiting.next;
		current->prev = NULL;
		waiting.next = current;
		current->next->prev = current;
		if (waiting.prev == NULL)
			waiting.prev = current;

		//Choose next runner:

		//Remove next runner from list:
		current = waiting.prev;
		current->prev->next = current->next;
		waiting.prev = current->prev;

		sigset_t set;
		sigfillset(&set);
		sigprocmask(SIG_UNBLOCK, &set, NULL);
		restorectx(&(current->ctx), 1);
	}
}

int sched_fork(){
	struct sched_proc childProc;

	void *childsp;
	childsp = mmap(0, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	memcpy(childsp, current->sp, STACK_SIZE);

	struct savectx childCtx;
	if (savectx(&childCtx) == 0){
		void *lim1 = current->sp;
		void *lim0 = current->sp + STACK_SIZE;
		unsigned long adj = childsp - current->sp;
		adjstack(lim0, lim1, adj);

		childCtx.regs[JB_BP] = childCtx.regs[JB_BP] + adj;
		childCtx.regs[JB_SP] = childCtx.regs[JB_SP] + adj;

		childProc.pid = 2; childProc.ppid = current->pid;
		childProc.sp = childsp;
		childProc.ctx = childCtx;
		childProc.nice = current->nice;
		childProc.Name = "Child";

		current->retVal = childProc.pid;
		childProc.retVal = 0;

		childProc.next = waiting.next;
		childProc.prev = NULL;
		waiting.next = &childProc;
		if (waiting.prev == NULL)
			waiting.prev = &childProc;

		sched_switch();
	}

	return current->retVal;
}

void sched_exit(int code){

}

int sched_wait(){

}

void sched_nice(int niceval){
	if (niceval < -20) niceval = -20;
	else if (niceval > 19) niceval = 19;
	current->nice = niceval;
}

int sched_getpid(){
	//return current;
}
int sched_ppid(){

}

void sched_ps(){

}

void sched_tick(int sig){
	printf("HELLO\n");
	sched_switch();
}

void sched_init(void (*init_fn)()){
	waiting.next = NULL; waiting.prev = NULL;

	struct itimerval curr_value, old_value;
	curr_value.it_interval.tv_sec = 0;
	curr_value.it_interval.tv_usec = 1000*TIMER_MSECS;
	curr_value.it_value.tv_sec = 0;
	curr_value.it_value.tv_usec = 1000*TIMER_MSECS;
	setitimer(ITIMER_VIRTUAL, &curr_value, &old_value);

	signal(SIGVTALRM, sched_tick);
	signal(SIGABRT, sched_ps);

	struct sched_proc firstProc;

	firstProc.pid = 1; firstProc.ppid = 0;
	void *newsp;
	newsp = mmap(0, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	firstProc.sp = newsp;
	firstProc.nice = 0;
	firstProc.next = NULL; firstProc.prev = NULL;
	firstProc.Name = "Parent";

	struct savectx initCtx;
	initCtx.regs[JB_SP] = newsp + STACK_SIZE;
	initCtx.regs[JB_BP] = newsp;
	initCtx.regs[JB_PC] = init_fn;
	current = &firstProc;
	restorectx(&initCtx, 0);
}