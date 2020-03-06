#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lock.h"
#include <stdio.h>

int DEFAULT_LOCK_PRIO = 2;

void dotask(char msg) {
    int i;
    kprintf("  %c: to start long time job\n", msg);
    for (i = 0; i < 2000000000; i++) ;
    kprintf("  %c: finished long time job\n", msg);
}

void getsem(char msg, int mutex) {
    kprintf("  %c: to acquire mutex\n", msg);
    wait(mutex);
    kprintf("  %c: acquired mutex, sleep 3s\n", msg);
    sleep(3);
    kprintf("  %c: to release mutex\n", msg);
    signal(mutex);
}


void testsem() {
    int mutex;
    int a, b, c;
    mutex = screate(1);

    a = create(getsem, 2000, 10, "A", 2, 'A', mutex);
    b = create(dotask, 2000, 20, "B", 1, 'B');
    c = create(getsem, 2000, 30, "C", 2, 'C', mutex); 
    
    kprintf("\nTest on semaphore.\n");
    kprintf("A & C try to grab mutex, C(30) is blocked by A(10)\n");
    kprintf("B is doing long time job, A(10) is blocked by B(20)\n");

    kprintf("-start A(10), then sleep 1s. mutex granted to A\n");
    resume(a);
    sleep(1);

    kprintf("-start C(30), then sleep 1s. C waits for mutex\n");
    resume(c);
    sleep(1);

    kprintf("-start B(20), B will do a long time job\n");
    resume(b);

}

void writer (char msg, int lck)
{
	kprintf ("  %c: to acquire lock\n", msg);
    lock (lck, WRITE, DEFAULT_LOCK_PRIO);
    kprintf ("  %c: acquired lock, sleep 3s\n", msg);
    sleep (3);
    kprintf ("  %c: to release lock\n", msg);
    releaseall (1, lck);
}

void testlock ()
{
    int     lck;
    int     a, b, c;

    lck = lcreate();

    a = create(writer, 2000, 10, "A", 2, 'A', lck);
    b = create(dotask, 2000, 20, "B", 1, 'B');
    c = create(writer, 2000, 30, "C", 2, 'C', lck); 
    
    kprintf("\nTest on lock.\n");
    kprintf("A & C try to grab lock, C(30) is blocked by A(10)\n");
    kprintf("B is doing long time job\n");
    kprintf("Priority inheritance will boost A(10) to A(30), A(30) will not blocked by B(20)\n");

    kprintf("-start A(10), then sleep 1s. lock granted to A\n");
    resume(a);
    sleep(1);

    kprintf("-start C(30), then sleep 1s. C waits for lock\n");
    resume(c);
    sleep(1);

    kprintf("-start B(20), B will do a long time job\n");
    resume(b);

}

