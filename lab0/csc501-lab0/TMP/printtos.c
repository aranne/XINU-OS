#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

unsigned long *fp = 0;

void printtos() {
    asm("movl %ebp, fp");

    kprintf("void printtos()\n");
    // Print address of top stack before get into printtos()
    kprintf("Before[0x%08x]: 0x%08x\n", fp+2, *(fp+2));
    // Print address of top stack after get into printtos()
    kprintf("After [0x%08x]: 0x%08x\n", fp, *fp);

    // Find base address of current process
    int pid = currpid;
    struct pentry *proc = &proctab[pid];
    unsigned long *base = (unsigned long*) proc->pbase;

    int k = 0;
    while (k < 4 && fp < base) {
        kprintf("\telement[0x%08x]: 0x%08x\n", fp, *fp);
        ++fp;
        ++k;
    }
    kprintf("\n");
}

