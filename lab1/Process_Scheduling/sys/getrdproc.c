#include <stdio.h>
#include <kernel.h>
#include <q.h>
#include "lab1.h"

/*-------------------------------------------------------------------------
 * getrandproc --  get the random process in a list according to random num
 *-------------------------------------------------------------------------
 */
int getrandproc(int tail, int random) {
    int proc = q[tail].qprev;

    if (proc >= NPROC) {
        return EMPTY;
    }

    while (q[proc].qkey <= random) {
        random -= q[proc].qkey;
        proc = q[proc].qprev;
    }
    return dequeue(proc);
}