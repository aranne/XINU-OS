/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;
	disable(ps);

	struct pentry *pptr = &proctab[currpid];
	struct mblock *p, *q;
	unsigned top;

	if (size == 0 || pptr->hasvhp == NOVIRTUALHEAP 
	 	|| ((unsigned) block) >= ((unsigned) (pptr->vhpno + pptr->vhpnpages) * NBPG) 
		|| ((unsigned) block) < ((unsigned) pptr->vhpno)) {
		restore(ps);
		return SYSERR;
	}
	size = (unsigned)roundmb(size);
	for ( p = pptr->vmemlist->mnext, q = pptr->vmemlist;
		  p != (struct mblock *) NULL && p < block ;
		  q = p, p = p->mnext)
		;
	if (((top=(unsigned)q + q->mlen) > (unsigned)block && q != pptr->vmemlist)
		|| (p != NULL && (size+(unsigned)block) > (unsigned)p) ) {
		restore(ps);
		return SYSERR;
	}
	if ( q != pptr->vmemlist && top == (unsigned) block) {
		q->mlen += size;
	} else {
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}

	// free corresponding frames.
	int vpno = (unsigned)block / NBPG;
	int npages;
	if (size % NBPG != 0) {
		npages = size / NBPG + 1;
	} else {
		npages = size / NBPG;
	}
	int i;
	int frmno, store, pageth;
	if (bsm_lookup(currpid, vpno, &store, &pageth) == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	if (store != pptr->store) {
		restore(ps);
		return SYSERR;
	}
	for (i = 0; i < npages; i++) {
		if (lookup_frm(currpid, vpno, &frmno) == OK) {
			free_frm(currpid, frmno, store, pageth + i);
		}
		vpno++;
	}

	restore(ps);
	return(OK);
}
