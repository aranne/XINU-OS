#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

unsigned long *esp;

void printprocstks(int priority) {

    struct pentry *proc;
    int i;
    // NPROC is max num of processes
    for (i = 0; i < NPROC; i++) {
        proc = &proctab[i];
        if (proc->pstate != PRFREE && proc->pprio > priority) {
            if (proc->pstate == PRCURR) {
                asm("movl %esp, esp");
            } else {
                esp = (unsigned long*) proc->pesp;
            }
            kprintf("Stack Base: 0x%08x\n", proc->pbase);
            kprintf("Stack Size: 0x%08x\n", proc->pstklen);
            kprintf("Actual Stack Size: 0x%08x\n", proc->pbase - (WORD) esp);
            kprintf("Stack Limit: 0x%08x\n", proc->plimit);
            kprintf("Stack Pointer: 0x%08x\n", esp);
            kprintf("Process Name: %s\n", proc->pname);
            kprintf("Process ID: %d\n", i);
            kprintf("Process Priority: %d\n\n", proc->pprio);
        }
    }
}