#include <stdio.h>
#include "lab1.h"

int get_rand(int range) {
    if (range == 0) return -1;  // for null process
    return rand() % range;
}