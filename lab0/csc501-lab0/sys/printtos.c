#include <stdio.h>
#include "lab0.h"

unsigned long *sp, *fp;

void printtos() {
    asm("movl %esp, sp");
    asm("movl %ebp, fp");

    // Print address of top stack before get into printtos()
    kprintf("Before: 0x%08x\n", fp+2);
    // Print address of top stack after get into printtos()
    kprintf("After: 0x%08x\n", fp);

    int k = 0;
    while (k < 4 && sp < fp) {
        kprintf("Content: 0x%08x\n", *sp);
        ++sp;
        ++k;
    }
    kprintf("\n");
}

