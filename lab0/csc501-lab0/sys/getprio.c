/* getprio.c - getprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * getprio -- return the scheduling priority of a given process
 *------------------------------------------------------------------------
 */
SYSCALL getprio(int pid)
{
	unsigned long start = ctr1000;
	if (tracking) {
		pCall[currpid] = 1;
		freq[currpid][3]++;
	}

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		if (tracking) {
			time[currpid][3] += ctr1000 - start;
		}
		return(SYSERR);
	}
	restore(ps);

	if (tracking) {
		time[currpid][3] += ctr1000 - start;
	}

	return(pptr->pprio);
}
