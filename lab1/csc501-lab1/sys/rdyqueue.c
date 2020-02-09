#include <stdio.h>
#include <q.h>
#include <kernel.h>
#include "lab1.h"

int nrdyproc = 0;
int sumpprio = 0;

void info_rdyproc() {
    int next;
    nrdyproc = 0;
    sumpprio = 0;

    next = q[rdyhead].qnext;
    while (next != rdytail) {
        nrdyproc++;
        sumpprio += q[next].qkey;
        next = q[next].qnext;
    }
    return;
}