#include <kernel.h>
#include <proc.h>
#include "lock.h"

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */

SYSCALL ldelete(int ldes) {
    STATWORD ps;
    int pid;
    struct lentry *lptr;

    disable(ps);
    if (!validlock(ldes)) {
        restore(ps);
        return SYSERR;
    }
    int lock = ldes / NLOCK;
    int i;
    /* free lock from process holding it */
    for (i = 0; i < NPROC; i++) {
        if (haslock(lock, i)) {
            releaselock(lock, i);
        }
    }
    lptr = &locktab[lock];
    lptr->lstate = LFREE;
    /* free lock from process waiting it */
    if (nonempty(lptr->lqhead)) {
        while ( (pid = getfirst(lptr->lqhead)) != EMPTY ) {
            proctab[pid].plockret = DELETED;
            ready(pid, RESCHNO);
        }
        resched();
    }
    restore(ps);
    return OK;
}