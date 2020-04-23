/* pfintr.S - pfintr */


    	   .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
    popl pferrcode    /* pop off page fault error code */
    pushfl            /* push EFLAGS */
    cli               /* diable further interrupt */
    pushal            /* push all general registers onto stack */
    call pfint        /* call page fault handler */
    popal             /* pop all general registers */
    popfl             /* pop EFLAGS */
    iret              /* interrupt return */




