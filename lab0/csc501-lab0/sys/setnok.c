/* setnok.c - setnok */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  setnok  -  set next-of-kin (notified at death) for a given process
 *------------------------------------------------------------------------
 */
SYSCALL	setnok(int nok, int pid)
{
	unsigned long start = ctr1000;
	if (tracking) {
		pCall[currpid] = 1;
		freq[currpid][14]++;
	}

	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid)) {
		restore(ps);
		if (tracking) {
			time[currpid][14] += ctr1000 - start;
		}
		return(SYSERR);
	}
	pptr = &proctab[pid];
	pptr->pnxtkin = nok;
	restore(ps);

	if (tracking) {
		time[currpid][14] += ctr1000 - start;
	}
	return(OK);
}
