/* gettime.c - gettime */

#include <conf.h>
#include <kernel.h>
#include <date.h>
#include <proc.h>
#include "lab0.h"

extern int getutim(unsigned long *);

/*------------------------------------------------------------------------
 *  gettime  -  get local time in seconds past Jan 1, 1970
 *------------------------------------------------------------------------
 */
SYSCALL	gettime(long *timvar)
{
    unsigned long start = ctr1000;
    if (tracking) {
		pCall[currpid] = 1;
		freq[currpid][4]++;
	}
    /* long	now; */

	/* FIXME -- no getutim */

	if (tracking) {
		time[currpid][4] += ctr1000 - start;
	}

    return OK;
}
