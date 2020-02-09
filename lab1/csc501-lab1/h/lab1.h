#ifndef _lab1_h
#define _lab1_h

#define RANDOMSCHED 1
#define LINUXSCHED 2
#define DEFAULTSCHED 3

extern int SCHEDCLASS;
extern int nrdyproc;
extern int sumpprio;

/* set schedule mode */
void setschedclass(int sched_class);
int getschedclass();

/* get number of processes and sum of their priority in ready queue*/
void info_rdyproc();

/* get random number in range 0 ~ sumpprio */
int get_rand(int range);

/* get the random process in a list according to random num */
int getrandproc(int tail, int random);

/* print processes in current and ready state */
void printproc();

#endif