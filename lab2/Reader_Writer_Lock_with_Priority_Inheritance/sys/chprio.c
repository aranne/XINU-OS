/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0
	    || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	int prevprio = getinhprio(pid);
	pptr->pprio = newprio;
	if (pptr->pinh <= newprio) {
		pptr->pinh = 0;
	}
	if (prevprio != getinhprio(pid) && pptr->pstate == PRLWAIT) {
		int ldes = pptr->plbid;
		if (validlock(ldes)) {
			int lock = ldes / NLOCK;
			updatemaxwaitprio(lock);
			updateprio_heldprocs(lock);
		}
	}
	restore(ps);
	return(getinhprio(pid));
}
