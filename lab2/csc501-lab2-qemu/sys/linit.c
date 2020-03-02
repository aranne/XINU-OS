#include "lock.h"
#include "q.h"

struct lentry locktab[];
int nextlock;

void linit() {
    struct lentry *lptr;
    nextlock = NLOCK - 1;

    int i;
    for (i = 0; i < NLOCK; i++) {
        (lptr = &locktab[i])->lstate = LFREE;
        lptr->lversion = 0;                   /* init version is 0 */
        lptr->lqtail = 1 + (lptr->lqhead = newqueue());
    }
}