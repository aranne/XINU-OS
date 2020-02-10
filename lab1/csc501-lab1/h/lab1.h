#ifndef _lab1_h
#define _lab1_h

#define RANDOMSCHED 1
#define LINUXSCHED 2
#define DEFAULTSCHED 3

extern int SCHEDCLASS;

/* set schedule mode */
void setschedclass(int sched_class);
int getschedclass();

/* get number of processes */
int numrdy();
/* and get sum of their priority in ready queue */
int sumrdyprio();

/* get random number in range 0 ~ sumpprio */
int get_rand(int range);

/* get the random process in a list according to random num */
int getrandproc(int tail, int random);

/* check an epoch ends or not */
int isEpochEnd();

/* start a new epoch */
void newEpoch();

/* get best process in ready queue */
int getbestproc(int tail);

/* print processes in current and ready state */
void printproc();

#endif