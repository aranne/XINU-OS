#include <stdio.h>
#include <q.h>
#include <kernel.h>
#include "lab1.h"


int sumrdyprio() {
    int next;
    int sumpprio = 0;

    next = q[rdyhead].qnext;
    while (next != rdytail) {
        sumpprio += q[next].qkey;
        next = q[next].qnext;
    }
    return sumpprio;
}

int numrdy() {
    int next;
    int num = 0;
    
    next = q[rdyhead].qnext;
    while (next != rdytail) {
        num++;
        next = q[next].qnext;
    }
    return num;
}