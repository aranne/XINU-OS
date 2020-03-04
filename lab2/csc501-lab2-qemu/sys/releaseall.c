#include <kernel.h>
#include <proc.h>
#include "lock.h"

int assignnext(int lock);                     /* assign lock to next process */

SYSCALL releaseall (int numlocks, int ldes1, ...) {
    int nargs = numlocks;
    unsigned int *a = (unsigned int *) &ldes1;     /* points to list of args */
    int ldes, lock;
    struct lentry *lptr;

    STATWORD ps;
    disable(ps);
    int ret = OK;
    int needresch = FALSE;
    for ( ; nargs > 0; nargs--) {
        ldes = *a++;
        lock = ldes / NLOCK;
        if (!validlock(ldes) || !haslock(lock, currpid)) {
            ret = SYSERR;
            continue;
        }
        lptr = &locktab[lock];
        releaselock(lock);
        if (isotherread(lock)) {
            lptr->lockcnt--;
        } else {
            lptr->lockcnt = 0;
            int assign = assignnext(lock);
            if (assign) {
                needresch = TRUE;
            }
        }
    }
    if (needresch) {
        resched();
    }
    restore(ps);
    return ret;
}


void releaselock(int lock) {
    struct lentry *lptr = &locktab[lock];
    lptr->lprocs &= ~(1LL << currpid);
    proctab[currpid].plholds &= ~(1LL << lock);
}

int assignnext(int lock) {
    struct lentry *lptr = &locktab[lock];
    if (isempty(lptr->lqhead)) return FALSE;

    int max[2];
    maxwrite(max, lock);
    int wakeread = FALSE;
    int proc = q[lptr->lqtail].qprev;
    /* try to wake up readers */
    int nextproc;
    while (proc < NPROC) {
        if (proctab[proc].plbtype == READ 
            && (q[proc].qkey > max[0] 
                || (q[proc].qkey == max[0] 
                     && proctab[proc].plbtime <= proctab[max[1]].plbtime + 40)) ) {
            nextproc = q[proc].qprev;  // preserve next proc 
            wakeread = TRUE;
            dequeue(proc);
            acquirelock(lock, READ, proc);
            ready(proc, RESCHNO);
            proc = nextproc;
        } else {
            proc = q[proc].qprev;
        }     
    }
    /* try to wake up writers */
    if (!wakeread) {
        dequeue(max[1]);
        acquirelock(lock, WRITE, max[1]);
        ready(max[1], RESCHNO);
    }
    return TRUE;
}

SYSCALL releasealllock(int proc) {
    STATWORD ps;
    disable(ps);
    int lock;
    for (lock = 0; lock < NLOCK; lock++) {
        if (haslock(lock, proc)) {
            releaseall(1, lock * NLOCK + locktab[lock].lversion);
        }
    }
    restore(ps);
    return OK;
}