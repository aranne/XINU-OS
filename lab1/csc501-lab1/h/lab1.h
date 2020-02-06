#ifndef _lab1_h
#define _lab1_h

#define RANDOMSCHED 1
#define LINUXSCHED 2
#define DEFAULTSCHED 3

extern int SCHEDCLASS;

void setschedclass(int sched_class);
int getschedclass();

#endif