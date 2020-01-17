/* getpid.c - getpid */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <proc.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 * getpid  --  get the process id of currently executing process
 *------------------------------------------------------------------------
 */
SYSCALL getpid()
{
	unsigned long start = ctr1000;
	if (tracking) {
		pCall[currpid] = 1;
		freq[currpid][2]++;
	}

	if (tracking) {
		time[currpid][2] += ctr1000 - start;
	}
	return(currpid);
}
