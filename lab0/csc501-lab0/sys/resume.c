/* resume.c - resume */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * resume  --  unsuspend a process, making it ready; return the priority
 *------------------------------------------------------------------------
 */
SYSCALL resume(int pid)
{
	unsigned long start = ctr1000;
	if (tracking) {
		pCall[currpid] = 1;
		freq[currpid][9]++;
	}

	STATWORD ps;    
	struct	pentry	*pptr;		/* pointer to proc. tab. entry	*/
	int	prio;			/* priority to return		*/

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate!=PRSUSP) {
		restore(ps);
		if (tracking) {
			time[currpid][9] += ctr1000 - start;
		}
		return(SYSERR);
	}
	prio = pptr->pprio;
	ready(pid, RESCHYES);
	restore(ps);

	if (tracking) {
		time[currpid][9] += ctr1000 - start;
	}
	return(prio);
}
