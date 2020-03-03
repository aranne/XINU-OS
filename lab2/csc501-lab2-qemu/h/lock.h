#ifndef _lock_h
#define _lock_h

#include <kernel.h>
#include <q.h>

#define NLOCK    50     /* number of locks   */     

#define LFREE  '\01'    /* this lock is free */
#define LUSED  '\02'    /* this lock is used */

#define READ    0
#define WRITE   1

struct lentry {    /* lock entry                     */
    char lstate;   /* the state LFREE or LUSED       */
    int lversion;  /* the version of this lock       */
    /* count > 0 means read, count == -1 means write, count == 0 means free */
    int lockcnt;   /* count of this lock             */
    int lqhead;    /* q index of head of list        */
    int lqtail;    /* q index of tail of list        */
    unsigned long long lprocs; /*  processes holding this lock */
    int lmaxprio;   /* max priority of processes holding this lock */
};

extern struct lentry locktab[];
extern int nextlock;
extern unsigned long ctr1000;

void linit(void);     /* initilize locks */
SYSCALL lcreate(void);     /* create a lock   */
SYSCALL ldelete(int ldes); /* delete a lock according to description */
SYSCALL lock(int ldes, int type, int priority); /* acquire a lock   */
SYSCALL releaseall (int numlocks, int ldes1, ...); /* release specific locks */

int validlock(int ldes);                      /* check validation of lock  */
int haslock(int lock);                        /* check current process has this lock */
int acquirelock(int lock, int type, int proc);            /* acquire this lock                   */
int waitlock(int ldes, int priority, int type); /* wait on this lock                   */
void releaselock(int lock);                    /* release a lock              */
void maxwrite(int* max, int lock);                    /* find the max priority of a writer    */
void printproc(void);
void printqueue(int tail);


#define isbadlock(l) (l<0 || l>=NLOCK)
#define isidle(l) (locktab[(l)].lockcnt == 0)
#define isread(l) (locktab[(l)].lockcnt > 0)
#define isotherread(l) (locktab[(l)].lockcnt > 1)
#define iswrite(l) (locktab[(l)].lockcnt == -1)

#endif