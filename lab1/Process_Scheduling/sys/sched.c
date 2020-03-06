#include <stdio.h>
#include "lab1.h"

int SCHEDCLASS = DEFAULTSCHED;

void setschedclass(int sched_class) {
    SCHEDCLASS = sched_class;
}

int getschedclass() {
    return SCHEDCLASS; 
}