#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lock.h"
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}

/*--------------------------------Test 1--------------------------------*/
 
void reader1 (char *msg, int lck)
{
	lock (lck, READ, DEFAULT_LOCK_PRIO);
	kprintf ("  %s: acquired lock, sleep 2s\n", msg);
        
	sleep (2);
	kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void test1 ()
{
	int	lck;
	int	pid1;
	int	pid2;

	kprintf("\nTest 1: readers can share the rwlock\n");
	lck  = lcreate ();
	assert (lck != SYSERR, "Test 1 failed");

	pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
	pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

	resume(pid1);
	resume(pid2);
        int lock = lck / NLOCK;
        kprintf("%d\n", lock);
        assert (locktab[lock].lprocs == ((1LL<<pid1)|(1LL<<pid2)), "Failed\n");
        assert (proctab[pid1].plholds == (1LL<<lock), "failed1\n");
        assert (proctab[pid2].plholds == (1LL<<lock), "failed2\n");
	
	sleep (5);
	ldelete (lck);
        assert (locktab[lock].lprocs == 0, "Failed\n");
        assert (proctab[pid1].plholds == 0, "failed1\n");
        assert (proctab[pid2].plholds == 0, "failed2\n");
	kprintf ("Test 1 ok\n");
}

/*----------------------------------Test 2---------------------------*/
char output2[10];
int count2;
void reader2 (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, READ, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
	releaseall (1, lck);
}

void writer2 (char msg, int lck, int lprio)
{
	kprintf ("  %c: to acquire lock\n", msg);
        lock (lck, WRITE, lprio);
        output2[count2++]=msg;
        kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        sleep (3);
        output2[count2++]=msg;
        kprintf ("  %c: to release lock\n", msg);
        releaseall (1, lck);
}

void test2 ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1;

        kprintf("\nTest 2: wait on locks with priority. Expected order of"
		" lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

	rd1 = create(reader2, 2000, 20, "A", 3, 'A', lck, 20);
	rd2 = create(reader2, 2000, 20, "B", 3, 'B', lck, 30);
	rd3 = create(reader2, 2000, 20, "D", 3, 'D', lck, 25);
	rd4 = create(reader2, 2000, 20, "E", 3, 'E', lck, 20);
        wr1 = create(writer2, 2000, 20, "C", 3, 'C', lck, 28);
	
        kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
        resume(rd1);
        sleep (1);

        kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
        resume(wr1);
        sleep10 (1);


        kprintf("-start reader B, D, E. reader B is granted lock.\n");
        resume (rd2);
	resume (rd3);
	resume (rd4);

        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"ABABCCDEED",10)==0,"Test 2 FAILED\n");
        kprintf ("Test 2 OK\n");
}

/*----------------------------------Test 3---------------------------*/
void reader3 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void writer3 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test3 ()
{
        int     lck;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 3: test the basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test 3 failed");

        rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
        rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
        wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 3 failed");
	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed");

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert(getprio(wr1) == 20, "Test 3 failed");

        sleep (8);
        kprintf ("Test 3 OK\n");
}

void testinit() {
        int i;
        for (i = 0; i < NLOCK; i++) {
                struct lentry *lptr = &locktab[i];
                if (lptr->lstate != LFREE) {
                        kprintf("%d\n", i);
                }
                if (lptr->lversion != 0) {
                        kprintf("%d\n", i);
                }
        }
}

void testcreate() {
        /* more than NLOCK locks */
        int i;
        int ldes;
        for (i = 0; i < NLOCK; i++) {
                ldes = lcreate();
        }
        int f = lcreate();
        assert (f == SYSERR, "Fail\n");

        /* different version */
        int pver = locktab[ldes/NLOCK].lversion;
        ldelete(ldes);
        int new = lcreate();
        int nver = locktab[new/NLOCK].lversion;
        assert(ldes/NLOCK == new/NLOCK, "fail1\n");
        assert(pver + 1 == nver, "fail2\n");

}

void readerd (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        int status = lock (lck, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        kprintf ("  %c: to release lock\n", msg);
	status = releaseall (1, lck);
        if (status == OK) {
                kprintf ("  %c: release lock\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        sleep (3);
        kprintf ("  %c: to acquire lock\n", msg);
        status = lock (lck, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        

        kprintf ("  %c: to release lock\n", msg);
	status = releaseall (1, lck);
        if (status == OK) {
                kprintf ("  %c: release lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
}

void testdelete() {
        int rd1, wr1;
        int lck = lcreate();
        rd1 = create(readerd, 2000, 20, "A", 3, 'A', lck, 20);
        wr1 = create(writer2, 2000, 20, "W", 3, 'W', lck, 20);
        resume(wr1);
        resume(rd1);
        ldelete(lck);
        kprintf("deleting lock\n");
}

void readerv (char msg, int lck, int lprio)
{
        int     ret;

        kprintf ("  %c: to acquire lock\n", msg);
        int status = lock (lck, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        kprintf ("  %c: to release lock\n", msg);
	status = releaseall (1, lck);
        if (status == OK) {
                kprintf ("  %c: release lock\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        sleep(3);
        kprintf ("  %c: to acquire lock\n", msg);
        status = lock (lck, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        kprintf ("  %c: to release lock\n", msg);
	status = releaseall (1, lck);
        if (status == OK) {
                kprintf ("  %c: release lock\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
}

void testversion() {
        int i;
        int ldes;
        for (i = 0; i < NLOCK-1; i++) {
                ldes = lcreate();
        }
        int lck = lcreate();
        kprintf("lockid:%d\n", lck/NLOCK);
        int rd1, wr1;
        rd1 = create(readerv, 2000, 20, "A", 3, 'A', lck, 20);
        wr1 = create(writer2, 2000, 20, "W", 3, 'W', lck, 20);
        resume(wr1);
        resume(rd1);
        ldelete(lck);
        lck = lcreate();
        kprintf("lockid:%d\n", lck/NLOCK);
}

void trelease(char msg, int slock1, int slock2, int xlock1, int lprio) {
        kprintf ("  %c: to acquire lock\n", msg);
        int status = lock (slock1, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        status = lock (slock2, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        status = lock (xlock1, WRITE, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        int lock;
        for (lock = 0; lock < NLOCK; lock++) {
                if (haslock(lock, currpid)) {
                        kprintf("lock: %d\n", lock);
                }
        }
        kprintf("start releasing locks\n");
        releasealllock(currpid);
        for (lock = 0; lock < NLOCK; lock++) {
                if (haslock(lock, currpid)) {
                        kprintf("lock: %d\n", lock);
                }
        }
}

void testrelease() {
        int pid;
        int slock1 = lcreate();
        int slock2 = lcreate();
        int xlock1 = lcreate();
        pid = create(trelease, 2000, 20, "A", 5, 'A', slock1, slock2, xlock1, 20);
        resume(pid);
}

void tkill(char msg, int slock1, int slock2, int xlock1, int lprio) {
        kprintf ("  %c: to acquire lock\n", msg);
        int status = lock (slock1, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        status = lock (slock2, READ, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        status = lock (xlock1, WRITE, lprio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        int lock;
        for (lock = 0; lock < NLOCK; lock++) {
                if (haslock(lock, currpid)) {
                        kprintf("lock: %d\n", lock);
                }
        }
        sleep(5);
        for (lock = 0; lock < NLOCK; lock++) {
                if (haslock(lock, currpid)) {
                        kprintf("lock: %d\n", lock);
                }
        }
}

void testkill() {
        int pid;
        int slock1 = lcreate();
        int slock2 = lcreate();
        int xlock1 = lcreate();
        pid = create(tkill, 2000, 20, "A", 5, 'A', slock1, slock2, xlock1, 20);
        resume(pid);
        kill(pid);
}

int main( )
{
        /* These test cases are only used for test purposes.
         * The provided results do not guarantee your correctness.
         * You need to read the PA2 instruction carefully.
         */
	testkill();
	

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call exists the QEMU process.
         */
        // shutdown();
}




