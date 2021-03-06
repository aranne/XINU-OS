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

extern unsigned long ctr1000;

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
char output2[15];
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
	assert (getprio(wr1) == 25, "Test 3 failed1");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        resume (rd2);
	sleep (1);
	assert (getprio(wr1) == 30, "Test 3 failed2");
	
	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep (1);
	assert (getprio(wr1) == 25, "Test 3 failed3");

	kprintf("-kill reader A, then sleep 1s\n");
	kill (rd1);
	sleep(1);
	assert(getprio(wr1) == 20, "Test 3 failed4");

        sleep (8);
        kprintf ("Test 3 OK\n");
}

void testlckorder ()
{
        count2 = 0;
        int     lck;
        int     rd1, rd2, rd3, rd4;
        int     wr1, wr;

        lck  = lcreate ();
        assert (lck != SYSERR, "Test 2 failed");

        wr = create(writer2, 2000, 20, "w", 3, 'w', lck, 10);
	rd1 = create(reader2, 2000, 20, "A", 3, 'A', lck, 20);
	rd2 = create(reader2, 2000, 20, "B", 3, 'B', lck, 25);
	rd3 = create(reader2, 2000, 20, "C", 3, 'C', lck, 25);
	rd4 = create(reader2, 2000, 20, "D", 3, 'D', lck, 30);
        wr1 = create(writer2, 2000, 20, "W", 3, 'W', lck, 25);

        kprintf("-start writer w, then sleep 1s. lock granted to writer w\n");
        resume(wr);
        sleep (1);
	
        kprintf("-start reader A. reader A waits for lock\n");
        resume(rd1);

        kprintf("-start writer W, sleep 0.4s. writer W waits for the lock\n");
        resume(wr1);
        sleep10(4);

        kprintf("-start reader B, C, D.\n");
        resume (rd2);
        sleep10(6);
	resume (rd3);
        sleep10(5);
	resume (rd4);
        printqueue(locktab[lck/NLOCK].lqtail);

        sleep (15);
        kprintf("output=%s\n", output2);
        assert(mystrncmp(output2,"wwDBBDWWCAAC",12)==0,"Test 2 FAILED\n");
        kprintf ("Test assign_lock_order OK\n");
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
        releaseall(3, slock1, slock2, xlock1);
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
        sleep(3);
        kprintf("wake up\n");
}

void testkill() {
        int pid, pid2;
        int slock1 = lcreate();
        int slock2 = lcreate();
        int xlock1 = lcreate();
        pid2 = create(writer2, 2000, 20, "W", 3, 'W', xlock1, 20);
        pid = create(tkill, 2000, 20, "A", 5, 'A', slock1, slock2, xlock1, 20);
        resume(pid2);
        resume(pid);
        assert(proctab[pid].pstate == PRLWAIT, "fail"); 
        assert(haslock(slock1/NLOCK, pid), "failed1");
        assert(haslock(slock2/NLOCK, pid), "failed2");
        assert(!haslock(xlock1/NLOCK, pid), "failed3");
        kill(pid);
        assert(!haslock(slock1/NLOCK, pid), "failed1");
        assert(!haslock(slock2/NLOCK, pid), "failed2");
        assert(!haslock(xlock1/NLOCK, pid), "failed3");
}

void tdelete2(char msg, int slock, int prio) {
        kprintf ("  %c: to acquire lock\n", msg);
        int status = lock (slock, READ, prio);
        if (status == OK) {
                kprintf ("  %c: acquired lock, sleep 3s\n", msg);
        } else if (status == SYSERR) {
                kprintf("  %c: lock is invalid\n", msg);
        } else if (status == DELETED) {
                kprintf("  %c: lock is deleted\n", msg);
        }
        sleep(3);
}

void testdelete2() {
        int pid;
        int slock = lcreate();
        pid = create(tdelete2, 2000, 20, "A", 3, 'A', slock, 20);
        resume(pid);
        ldelete(slock);
        assert(!haslock(slock/NLOCK, pid), "failed");
}

/*----------------------------------Test 4---------------------------*/
void reader4 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}
void reader41 (char *msg, int lck, int lck2)
{
        int     ret;
        kprintf ("  %s: to acquire lock2\n", msg);
        int status = lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        if (status == OK) kprintf ("  %s: acquired lock2\n", msg);
        if (status == SYSERR) kprintf("  %s: cannot get lock2\n", msg);
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        sleep(5);
        kprintf ("  %s: to release lock & lock2\n", msg);
        releaseall (2, lck, lck2);
}

void writer4 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test4 ()
{
        int     lck, lck2;
        int     rd1, rd2;
        int     wr1;

        kprintf("\nTest 4: test the basic priority inheritence\n");
        lck  = lcreate ();
        lck2 = lcreate();
        assert (lck != SYSERR, "Test 4 failed");

        rd1 = create(reader41, 2000, 20, "reader4", 3, "reader A", lck, lck2);
        rd2 = create(reader4, 2000, 30, "reader4", 2, "reader B", lck2);
        wr1 = create(writer4, 2000, 25, "writer4", 2, "writer", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 25, "Test 4 failed1");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        assert(haslock(lck2/NLOCK, rd1), "fail!!!");
        resume (rd2);
	sleep (1);
        assert (getprio(rd1) == 30, "Test 4 failed22");
	assert (getprio(wr1) == 30, "Test 4 failed2");
	
	kprintf("-kill writer, then sleep 1s\n");
        assert (proctab[wr1].pstate == PRSLEEP, "failed???");
	kill (wr1);
	sleep (1);
	assert (getprio(rd1) == 30, "Test 4 failed3");

	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep(1);
	assert(getprio(rd1) == 20, "Test 4 failed4");

        sleep (8);
        kprintf ("Test 4 OK\n");
}

/*----------------------------------Test 4---------------------------*/
void reader5 (char *msg, int lck)
{
        int     ret;

        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}
void reader51 (char *msg, int lck, int lck2)
{
        int     ret;
        kprintf ("  %s: to acquire lock2\n", msg);
        int status = lock (lck2, WRITE, DEFAULT_LOCK_PRIO);
        if (status == OK) kprintf ("  %s: acquired lock2\n", msg);
        if (status == SYSERR) kprintf("  %s: cannot get lock2\n", msg);
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock\n", msg);
        sleep(5);
        kprintf ("  %s: to release lock & lock2\n", msg);
        releaseall (2, lck, lck2);
}

void writer5 (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 10s\n", msg);
        sleep (10);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void test5 ()
{
        int     lck, lck2;
        int     rd1, rd2, rd3;
        int     wr1;

        kprintf("\nTest 5: test the basic priority inheritence\n");
        lck  = lcreate ();
        lck2 = lcreate();
        assert (lck != SYSERR, "Test 5 failed");

        rd1 = create(reader51, 2000, 10, "reader5", 3, "reader A", lck, lck2);
        rd2 = create(reader5, 2000, 30, "reader5", 2, "reader B", lck2);
        wr1 = create(writer5, 2000, 20, "writer5", 2, "writer", lck);
        rd3 = create(reader5, 2000, 40, "reader5", 2, "reader C", lck);

        kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
        resume(wr1);
        sleep (1);

        kprintf("-start reader A, then sleep 1s. reader A(prio 10) blocked on the lock\n");
        resume(rd1);
        sleep (1);
	assert (getprio(wr1) == 20, "Test 5 failed1");
        assert  (getprio(rd1) == 10, "fail----");

        kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
        assert(haslock(lck2/NLOCK, rd1), "fail!!!");
        resume (rd2);
	sleep (1);
        assert (getprio(rd1) == 30, "Test 5 failed22");
	assert (getprio(wr1) == 30, "Test 5 failed2");

        kprintf("-start reader C, then sleep 1s. reader C(prio 40) blocked on the lock\n");
        resume(rd3);
        sleep(1);
        assert (getprio(wr1) == 40, "Test 5 failed33");
        assert (getprio(rd1) == 30, "Test 5 failed34");
	
	kprintf("-kill writer, then sleep 1s\n");
        assert (proctab[wr1].pstate == PRSLEEP, "failed???");
	kill (wr1);
	sleep (1);
        assert (haslock(lck/NLOCK, rd1), "Pass lck failed");
	assert (getprio(rd1) == 40, "Test 5 failed3");

        kprintf("-kill rd3, then sleep 1s\n");
        kill(rd3);
        sleep(1);
        assert(getprio(rd1) == 30, "Test 5 failed44");

	kprintf("-kill reader B, then sleep 1s\n");
	kill (rd2);
	sleep(1);
	assert(getprio(rd1) == 10, "Test 5 failed4");

        sleep (8);
        kprintf ("Test 5 OK\n");
}

int main( )
{
        /* These test cases are only used for test purposes.
         * The provided results do not guarantee your correctness.
         * You need to read the PA2 instruction carefully.
         */
        // test1();
        // test2();
        // test3();
        // test4();
        // test5();
        // testlckorder();
        testsem();
        testlock();

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call exists the QEMU process.
         */
        shutdown();
}




