/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <paging.h>
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

	/* for demand paging */
	/* free all frames */
	fr_map_t *frm;
	int i;
	for (i = 0; i < NFRAMES; i++) {
		frm = &frm_tab[i];
		if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_PAGE && frm->fr_pid == pid) {
			int vp = frm->fr_vpno;
			int store, pageth;
			if (bsm_lookup(pid, vp, &store, &pageth) == OK) {
				free_frm(pid, i, store, pageth);
			} else {
				clear_frm(i);
			}
		}
	}

	/* free all backing stores */
	bs_map_t *bsm;
	bs_vp_t *bsvp, *prev;
	for (i = 0; i < NBS; i++) {
		bsm = &bsm_tab[i];
		if (bsm->bs_status != BSM_UNMAPPED) {
			prev = NULL;
			bsvp = bsm->bs_vp;
			while (bsvp->bs_pid != -1) {
				if (bsvp->bs_pid == pid) {
					if (prev != NULL) {
						prev->nextvp = bsvp->nextvp;
						freemem((struct mblock *)bsvp, sizeof(bs_vp_t));
						bsvp = prev->nextvp;
					} else {
						bs_vp_t *next = bsvp->nextvp;
						freemem((struct mblock *)bsvp, sizeof(bs_vp_t));
						bsm->bs_vp = next;
						bsvp = next;
					}
				} else {
					prev = bsvp;
					bsvp = prev->nextvp;
				}
			}
			release_bs(i);
		}
	}

	/* free process directory */
	unsigned long pdbr = proctab[pid].pdbr;
	int frmno = pdbr / NBPG - FRAME0;
	if (free_frm(pid, frmno, -1, -1) == SYSERR ) {
		restore(ps);
		return SYSERR;
	}

	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
