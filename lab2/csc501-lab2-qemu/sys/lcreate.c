#include <kernel.h>
#include "lock.h"

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a semaphore, returning its id
 *------------------------------------------------------------------------
 */
SYSCALL lcreate() {
    STATWORD ps;
    int ldes;       /* lock description */

    disable(ps);
    if ((ldes = newlock()) == SYSERR) {
        restore(ps);
        return SYSERR;
    }
    restore(ps);
    return ldes;
}

/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock() {

    struct lentry *lptr;
    int lock;
    int version;
    int i;

    for (i = 0; i < NLOCK; i++) {
        lock = nextlock--;
        if (nextlock < 0) {
            nextlock = NLOCK - 1;
        }
        if (locktab[lock].lstate == LFREE) {
            lptr = &locktab[lock];
            version = lptr->lversion;
            if (++version >= NLOCK) {
                version = 0;
            }
            lptr->lversion = version;
            lptr->lstate = LUSED;
            lptr->lockcnt = 0;
            lptr->lprocs = 0;
            lptr->lwaitprio = 0;
            /* return lock and version value as a pair */
            return lock * NLOCK + version;       /* lock description */
        }
    }
    return SYSERR;
}