#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include "lab1.h"

/* best = priority + quantum */
int getbestproc(int tail) {
    int best = 0;
    int pid = 0;
    struct pentry * pptr;

    int proc = q[tail].qprev;  

    if (proc >= NPROC) {
        return EMPTY;
    }

    for (; proc < NPROC; proc = q[proc].qprev) {
        pptr = &proctab[proc];
        if (pptr->pquantum <= 0) continue;
        int goodness = pptr->pprevprio + pptr->pquantum;
        if (best <= goodness) {  // round-robin: processes with same goodness
            best = goodness;
            pid = proc;
        } 
    }
    // kprintf("%s:%d\n", proctab[pid].pname, proctab[pid].pquantum);
    // kprintf("%s:%d \n",proctab[pid].pname, best);
    return dequeue(pid);
}