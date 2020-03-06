#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lab1.h"

int isEpochEnd() {
    int proc;
    struct pentry * pptr;
    for (proc = q[rdytail].qprev; proc < NPROC; proc = q[proc].qprev) {
        pptr = &proctab[proc];
        if (pptr->pquantum > 0) {
            return FALSE;
        }
    }
    return TRUE;
}

void newEpoch() {
    int i;
    struct pentry * pptr;
    for (i = 1; i < NPROC; i++) { // exclude null process
        pptr = &proctab[i];
        if (pptr->pstate != PRFREE) {
            if (pptr->pquantum > 0) {
                pptr->pquantum = pptr->pprio + (pptr->pquantum / 2);
            } else {
                pptr->pquantum = pptr->pprio;
            }
            pptr->pprevprio = pptr->pprio;     // use this priority in this epoch 
        }
    }
}
