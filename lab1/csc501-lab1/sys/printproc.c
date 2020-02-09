#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include "lab1.h"

void printproc() {
    struct pentry* proc;
    int i;
    for (i = 0; i < NPROC; i++) {
        proc = &proctab[i];
        if (proc->pstate == PRCURR || proc->pstate == PRREADY) {
            kprintf("Process [%s]\n", proc->pname);
            if (proc->pstate == PRCURR) {
                kprintf("\tpstate: PRCURR\n");
            } else {
                kprintf("\tpstate: PRREADY\n");
            }
            kprintf("\tpriority: %d\n", proc->pprio);
        }
    }
    kprintf("\n");
}