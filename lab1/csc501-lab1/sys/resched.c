/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lab1.h"

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
int default_sched(void);
int random_sched(void);
int linux_sched(void);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched() {
	if (SCHEDCLASS == DEFAULTSCHED) {
		return default_sched();
	} else if (SCHEDCLASS == RANDOMSCHED) {
		return random_sched();
	} else if (SCHEDCLASS == LINUXSCHED) {
		return linux_sched();
	} else {
		return SYSERR;
	}
}

int random_sched() {
	register struct pentry *optr;
	register struct pentry *nptr;

    if (numrdy() == 0) return OK;
	
	optr = &proctab[currpid];

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid, rdyhead, optr->pprio);
	}

	/* get sum of their priority */
	int sumpprio = sumrdyprio();
	/* get random number in range 0 ~ sumpprio */
    int random = get_rand(sumpprio);

	/* remove process according to random number */
    currpid = getrandproc(rdytail, random);
	nptr = &proctab[currpid];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}

int linux_sched() {
	return default_sched();
}

int default_sched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	/* no switch needed if current process priority higher than next*/

	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */

	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */
	currpid = getlast(rdytail);
	nptr = &proctab[currpid];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
