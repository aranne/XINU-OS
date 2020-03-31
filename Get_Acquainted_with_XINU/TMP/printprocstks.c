#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

unsigned long *esp = 0;

void printprocstks(int priority) {

    kprintf("void printprocstks(int priority)\n");

    struct pentry *proc;
    int i;
    for (i = 0; i < NPROC; i++) {
        proc = &proctab[i];
        if (proc->pstate != PRFREE && proc->pprio > priority) {  // if process on this slot exists
            if (proc->pstate == PRCURR) {  // if process is currently running
                asm("movl %esp, esp");
            } else {                       // if process is not running
                esp = (unsigned long*) proc->pesp;
            }
            kprintf("Process [%s]\n", proc->pname);
            kprintf("\tpid: %d\n", i);
            kprintf("\tpriority: %d\n", proc->pprio);
            kprintf("\tbase: 0x%08x\n", proc->pbase);
            kprintf("\tlimit: 0x%08x\n", proc->plimit);
            kprintf("\tlen: %d\n", proc->pstklen);
            kprintf("\tpointer: 0x %08x\n", esp);
        }
    }
    kprintf("\n");
}