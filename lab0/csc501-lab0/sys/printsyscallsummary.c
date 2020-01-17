#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include "lab0.h"

int tracking = 0;  // is tracking or not
int pCall[NPROC];
int freq[NPROC][NSYSCALL];      // record the frequence of each system call for each process
int time[NPROC][NSYSCALL];      // record the time of each system call for each process
char syscallName[NSYSCALL][NAMELEN] = {"sys_freemem", "sys_chprio", "sys_getpid", "sys_getprio", "sys_gettime", "sys_kill", "sys_receive", "sys_recvclr"
                                    , "sys_recvtim", "sys_resume", "sys_scount", "sys_sdelete", "sys_send", "sys_setdev", "sys_setnok"
                                    , "sys_screate", "sys_signal", "sys_signaln", "sys_sleep", "sys_sleep10", "sys_sleep100", "sys_sleep1000"
                                    , "sys_sreset", "sys_stacktrace", "sys_suspend", "sys_unsleep", "sys_wait"};

void printsyscallsummary() {
    
    int i, j;
    for (i = 0; i < NPROC; ++i) {
        if (!pCall[i]) continue;
        kprintf("Process [pid:%d]\n", i);
        for (j = 0; j < NSYSCALL; ++j) {
            if (!freq[i][j]) continue;
            kprintf("\tSyscall: %s, count: %d, average execution time: %d (ms)\n", syscallName[j], freq[i][j], time[i][j] / freq[i][j]);
        }
    }
}

void syscallsummary_start() {
    tracking = 1;
    int i, j;
    for (i = 0; i < NPROC; ++i) {
        for (j = 0; j < NSYSCALL; ++j) {
            freq[i][j] = 0;
            time[i][j] = 0;
        }
        pCall[i] = 0;
    }
}

void syscallsummary_stop() {
    tracking = 0;
}