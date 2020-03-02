#include <kernel.h>
#include "lock.h"

SYSCALL releaseall (int numlocks, int ldes1, ...) {
    int nargs = numlocks;
    unsigned int *a = (unsigned int *) &ldes1;     /* points to list of args */
    int ldes;

    STATWORD ps;
    disable(ps);
    int ret = OK;
    for ( ; nargs > 0; nargs--) {
        ldes = *a++;
        if (!validlock(ldes)) {
            ret = SYSERR;
            continue;
        }
        
    }

    return OK;
}