/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main()
{
	kprintf("\n\nHello World, Xinu@QEMU lives\n\n");
        init_bsm();
        bs_map_t *bsm = &bsm_tab[1];
        bsm_map(2, 1000, 1, 100, 0);
        bsm_map(currpid, 2000, 1, 50, 0);
        bsm_map(10, 3000, 1, 90, 0);
        kprintf("%d\n", bsm->bs_vp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->nextvp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->nextvp->nextvp->bs_pid);
        bsm_unmap(2, 1080, 0);
        bsm_unmap(10, 3080, 0);
        kprintf("%d\n", bsm->bs_vp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->nextvp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->nextvp->nextvp->bs_pid);
        release_bs(1);
        kprintf("%d\n", bsm->bs_vp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->nextvp->bs_pid);
        kprintf("%d\n", bsm->bs_vp->nextvp->nextvp->nextvp->bs_pid);

        // unsigned int pg_offset, pt_offset, pd_offset;
        // pg_offset = vaddr & 0xFFF;
        // pt_offset = (vaddr >> 12) & 0x3FF;
        // pd_offset = (vaddr >> 22) & 0x3FF;
        // pd_t* pd = proctab[pid].pdbr + pd_offset * 4;
        // pt_t* pt = pd->pd_base + pt_offset * 4;

        /* The hook to shutdown QEMU for process-like execution of XINU.
         * This API call terminates the QEMU process.
         */
        shutdown();
}
