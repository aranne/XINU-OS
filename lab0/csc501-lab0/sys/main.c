/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include "lab0.h"

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu lives\n\n");
	long x = zfunction(0xaabbccdd);
	kprintf("0x%0x\n\n", x);

	printsegaddress();

	printtos();

	printprocstks(0);

	return 0;
}
