#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include "lock.h"

int hasproc(int lock, int pid);

int getinhprio(int proc) {
    if (proctab[proc].pinh == 0) {
        return proctab[proc].pprio;
    } else {
        return proctab[proc].pinh;
    }
}

/* increase priority of processes holding this lock */ 
void updateprio_heldprocs(int lock) {
    int proc;
    for (proc = 0; proc < NPROC; proc++) {
        if (hasproc(lock, proc)) {
            updateprio_holdinglocks(proc);   
        }
    }
}

/* find the max priority of processes waiting for the lock */
/* sets 0 if no waiting process */
void updatemaxwaitprio(int lock) {
    int max = 0;
    int proc = q[locktab[lock].lqtail].qprev;
    while (proc < NPROC) {
        if (max < getinhprio(proc)) {
            max = getinhprio(proc);
        }
        proc = q[proc].qprev;
    }
    locktab[lock].lwaitprio = max;
}

/* find max wait priority of locks holding by process pid */ 
/* sets 0 if no holding locks */
void updateprio_holdinglocks(int pid) {
    int max = 0;
    int lock;
    for (lock = 0; lock < NLOCK; lock++) {
        if (haslock(lock, pid)) {
            if (max < locktab[lock].lwaitprio) {
                max = locktab[lock].lwaitprio;
            }
        }
    }
    int prevprio = getinhprio(pid);
    if (max <= proctab[pid].pprio) {
        proctab[pid].pinh = 0;
    } else {
        proctab[pid].pinh = max;
    }
    /* if priority changes & PRLWAIT, priority transition */
    if (prevprio != getinhprio(pid) && proctab[pid].pstate == PRLWAIT) {     
        int ldes = proctab[pid].plbid;
        if (validlock(ldes)) {
            int l = ldes / NLOCK;
            updatemaxwaitprio(l);
            updateprio_heldprocs(l);
        }
    }
}

int hasproc(int lock, int pid) {
    unsigned long long mask = locktab[lock].lprocs;
    unsigned long long test = (1LL << pid);
    return (mask & test) == test;
}