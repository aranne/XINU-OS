#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include "lock.h"

void printproc() {
    struct pentry* proc;
    int i;
    for (i = 0; i < NPROC; i++) {
        proc = &proctab[i];
        if (proc->pstate == PRCURR || proc->pstate == PRREADY || proc->pstate == PRLWAIT || proc->pstate == PRSLEEP) {
            kprintf("Process [%s]\n", proc->pname);
            if (proc->pstate == PRCURR) {
                kprintf("\tpstate: PRCURR\n");
            } else if (proc->pstate == PRREADY) {
                kprintf("\tpstate: PRREADY\n");
            } else if (proc->pstate == PRLWAIT) {
                kprintf("\tpstate: PRLWAIT\n");
            } else {
                kprintf("\tpstate: PRSLEEP\n");
            }
        }
    }
    kprintf("\n");
}

void printqueue(int tail) {
	int proc = q[tail].qprev;
	for ( ; proc < NPROC; proc = q[proc].qprev) {
		kprintf("Process [%s]:%d\n", proctab[proc].pname, proc);
        if (proctab[proc].plbtype == READ) {
            kprintf("\tplbtype: READ\n");
        } else {
            kprintf("\tplbtype: WRITE\n");
        }
        kprintf("\tplbtime: %ld\n", proctab[proc].plbtime);
	}
}