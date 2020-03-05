/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include "lock.h"
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			releasealllock(pid);
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;
			dequeue(pid);
			releasealllock(pid);
			pptr->pstate = PRFREE;
			break;
	case PRLWAIT:   dequeue(pid);
			if (validlock(pptr->plbid)) {
				updatemaxwaitprio(pptr->plbid / NLOCK);
				updateprio_heldprocs(pptr->plbid / NLOCK);
			}
			releasealllock(pid);
			pptr->pstate = PRFREE;
			break;
	case PRREADY:	dequeue(pid);
			releasealllock(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
					/* fall through	*/
	default:	
			releasealllock(pid);
			pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
