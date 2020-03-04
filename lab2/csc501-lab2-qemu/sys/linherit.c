#include <stdio.h>
#include <kernel.h>
#include <proc.h>
#include "lock.h"

int getinhprio(int proc) {
    if (proctab[proc].pinh == 0) {
        return proctab[proc].pprio;
    } else {
        return proctab[proc].pinh;
    }
}