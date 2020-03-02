/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lock.h"
extern unsigned long ctr1000;
extern int preempt;

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }

int main()
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call terminates the QEMU process.
         */
        // test1();
        releaseall(2, 1, 2);
        releaseall(4, 1, 2, 3, 4);
}

void acquire (char *msg, int sem)
{
	int status = wait(sem);
        if (status == OK) {
	    kprintf (" %s: acquired lock, sleep 2s\n", msg);
            sleep(2);
            kprintf(" %s: realse lock\n", msg);
            signal(sem);
        } else if (status == DELETED) {
            kprintf(" %s: lock has been deleted\n", msg);
        }
}

void test1 ()
{
	int	sem;
	int	pid1;
	int	pid2;

	sem  = screate(1);

	pid1 = create(acquire, 2000, 20, "reader a", 2, "reader a", sem);
	pid2 = create(acquire, 2000, 20, "reader b", 2, "reader b", sem);

	resume(pid1);
	resume(pid2);
        sdelete(sem);
}