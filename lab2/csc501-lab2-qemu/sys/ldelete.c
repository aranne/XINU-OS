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
    lptr = &locktab[lock];
    lptr->lstate = LFREE;
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