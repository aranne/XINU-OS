/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD 	ps;    
	int		pid;		/* stores new process id	*/
	struct	pentry	*pptr;		/* pointer to proc. table entry */
	struct mblock *mptr;

	disable(ps);

	pid = create(procaddr, ssize, priority, name, nargs, args);
	if (pid == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	pptr = &proctab[pid];

	/* for demand paging */
	pptr->hasvhp = HASVIRTUALHEAP;

	pptr->vhpno = VHSNO;    // starting virtual page number
	pptr->vhpnpages = hsize;

	int store;
	if (get_bsm(&store) == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	pptr->store = store;
	if (bsm_map(pid, pptr->vhpno, pptr->store, pptr->vhpnpages, 1) == SYSERR) {
		restore(ps);
		return SYSERR;
	}

	// pptr->vmemlist = (struct mblock *) getmem(sizeof(struct mblock));
	pptr->vmemlist = (struct mblock *) getmem(sizeof(struct mblock));
	pptr->vmemlist->mnext = VHSNO * NBPG;

	/* write mlen and mnext into corresponding backing store to avoid page fault interrupt */
	mptr = BACKING_STORE_BASE + store * BACKING_STORE_UNIT_SIZE;
	mptr->mnext = 0;
	mptr->mlen = hsize * NBPG;

	restore(ps);
	return(pid);
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
