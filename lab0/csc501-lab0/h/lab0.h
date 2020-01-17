#ifndef _lab0_h_
#define _lab0_h_

// Task 1:
long zfunction(long x);

// Task 2:
void printsegaddress(void);

// Task 3:
void printtos(void);

// Task 4:
void printprocstks(int priority);

// Task 5:
#ifndef NSYSCALL
#define NSYSCALL 27         // we need to track 27 system calls
#endif

#ifndef NPROC
#define NPROC 50
#endif

#ifndef NAMELEN
#define NAMELEN 15         // system call name max length
#endif

extern int tracking;  // is tracking or not
extern int pCall[NPROC];
extern int freq[NPROC][NSYSCALL];      // record the frequence of each system call for each process
extern int time[NPROC][NSYSCALL];      // record the time of each system call for each process
extern unsigned long ctr1000;	

void syscallsummary_start(void);
void syscallsummary_stop(void);
void printsyscallsummary(void);

#endif