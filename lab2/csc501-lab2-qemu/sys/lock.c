#include <kernel.h>
#include <proc.h>
#include "lock.h"

unsigned long ctr1000;

int slock(int ldes, int priority);
int xlock(int ldes, int priority);
int haslock(int lock);                           /* check current process has this lock */
int acquirelock(int lock, int type);            /* acquire this lock                   */
int waitlock(int ldes, int priority, int type); /* wait on this lock                   */
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
        if (!haslock(lock)) {
            getlock = slock(ldes, priority);
        }
    } else {
        if (!(haslock(lock) && iswrite(lock))) { // doesn't have write lock
            getlock = slock(ldes, priority);
            if (getlock == OK) {
                getlock  = xlock(ldes, priority);
            }
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

int haslock(int lock) {
    long long mask = proctab[currpid].plholds;
    long long test = 1 << lock;
    return mask & test == test;
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
        getlock = acquirelock(lock, READ);
    } else if (isread(lock) && canread(lock, priority)) {
        getlock = acquirelock(lock, READ);
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
    if (isotherread(lock)) { // has gotten slock.
        getlock = waitlock(ldes, priority, WRITE);
    } else {
        getlock = acquirelock(lock, WRITE);
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

int acquirelock(int lock, int type) {
    struct lentry *lptr = &locktab[lock];
    if (type == READ) {
        lptr->lockcnt++;
    } else {
        lptr->lockcnt = -1;
    }  
    lptr->lprocs |= (1 << currpid);
    proctab[currpid].plholds |= (1 << lock);
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
    int max = -1;
    int proc = q[locktab[lock].lqtail].qprev;

    /* find max priority of write process */
    for (; proc < NPROC; proc = q[proc].qprev) {
        if (proctab[proc].plbtype == WRITE) {
            max = q[proc].qkey;
            break;
        }
    }
    if (max == -1) return TRUE;
    if (priority > max) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int validlock(int ldes) {    /* check validation of lock  */
    int lock = ldes / NLOCK;
    int version = ldes % NLOCK;
    return (!isbadlock(lock) && locktab[lock].lstate != LFREE 
            && version == locktab[lock].lversion);
}