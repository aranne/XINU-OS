#include <kernel.h>
#include <proc.h>
#include "lock.h"

unsigned long ctr1000;

int slock(int ldes, int priority);
int xlock(int ldes, int priority);
int canread(int lock, int priority);  /* check if grace period for read */

/*------------------------------------------------------------------------
 * lock  --  acquire a lock for current process
 *------------------------------------------------------------------------
 */

SYSCALL lock(int ldes, int type, int priority) {

    struct lentry *lptr;
    struct pentry *pptr;

    int lock = ldes / NLOCK;
    int version = ldes % NLOCK;

    int getlock = OK;
    if (type == READ) {
        if (!haslock(lock, currpid)) {
            getlock = slock(ldes, priority);
        }
    } else {
        if (!(haslock(lock, currpid) && iswrite(lock))) { // doesn't have write lock
            if (haslock(lock, currpid)) {
                releaseall(1, ldes);
            }
            getlock = xlock(ldes, priority);
        }
    }
    if (getlock == OK) {
        return OK;
    } else if (getlock == DELETED){
        return DELETED; 
    } else {
        return SYSERR;
    }
}

int slock(int ldes, int priority) {
    STATWORD ps;
    struct	pentry	*pptr;

    disable(ps);
    if (!validlock(ldes)) {
        restore(ps);
        return SYSERR;
    }
    int getlock;
    int lock = ldes / NLOCK;
    if (isidle(lock)) {
        getlock = acquirelock(lock, READ, currpid);
    } else if (isread(lock) && canread(lock, priority)) {
        getlock = acquirelock(lock, READ, currpid);
    } else {
        getlock = waitlock(ldes, priority, READ);
    }
    if (getlock) {
        restore(ps);
        return OK;
    } else {
        pptr = &proctab[currpid];
        pptr->plockret = OK;
        resched();
        restore(ps);
        return pptr->plockret;
    }
}

int xlock(int ldes, int priority) {
    STATWORD ps;
    struct	pentry	*pptr;

    disable(ps);
    if (!validlock(ldes)) {
        restore(ps);
        return SYSERR;
    }
    int getlock;
    int lock = ldes / NLOCK;
    if (!isidle(lock)) { // has gotten slock.
        getlock = waitlock(ldes, priority, WRITE);
    } else {
        getlock = acquirelock(lock, WRITE, currpid);
    }
    if (getlock) {
        restore(ps);
        return OK;
    } else {
        pptr = &proctab[currpid];
        pptr->plockret = OK;
        resched();
        restore(ps);
        return pptr->plockret;
    }
}

int acquirelock(int lock, int type, int proc) {
    struct lentry *lptr = &locktab[lock];
    if (type == READ) {
        lptr->lockcnt++;
    } else {
        lptr->lockcnt = -1;
    }  
    lptr->lprocs |= (1LL << proc);
    proctab[proc].plholds |= (1LL << lock);
    return TRUE;
}

int waitlock(int ldes, int priority, int type) {
    int lock = ldes / NLOCK;
    struct lentry *lptr = &locktab[lock];
    struct pentry *pptr = &proctab[currpid];
    unsigned long time = ctr1000;

    if (type == READ) {
        pptr->plbtype = READ;
    } else {
        pptr->plbtype = WRITE;
    }
    pptr->plbid = ldes;
    pptr->plbtime = time;
    pptr->pstate = PRLWAIT;
    insert(currpid, lptr->lqhead, priority);
    return FALSE;
}

int canread(int lock, int priority) {
    int max[2];
    maxwrite(max, lock);
    if (max[0] == -1) return TRUE;
    if (priority > max[0]) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* return buffer max [-1,-1] means no writer */
void maxwrite(int* max, int lock) {
    max[0] = -1;      /* max priority */
    max[1] = EMPTY;   /* pid          */
    int proc = q[locktab[lock].lqtail].qprev;
    for ( ; proc < NPROC; proc = q[proc].qprev) {
        if (proctab[proc].plbtype == WRITE) {
            max[0] = q[proc].qkey;
            max[1] = proc;
            break;
        }
    }
}

int validlock(int ldes) {    /* check validation of lock  */
    int lock = ldes / NLOCK;
    int version = ldes % NLOCK;
    return (!isbadlock(lock) && locktab[lock].lstate != LFREE 
            && version == locktab[lock].lversion);
}

int haslock(int lock, int pid) {
    unsigned long long mask = proctab[pid].plholds;
    unsigned long long test = (1LL << lock);
    return (mask & test) == test;
}