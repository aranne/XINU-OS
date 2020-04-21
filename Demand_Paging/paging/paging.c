#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL create_pd(int pid) {
    STATWORD ps;
    disable(ps);

    if (pid < 0 || pid >= NPROC) {
        restore(ps);
        return SYSERR;
    }

    fr_map_t *frm;
    int frmno;
    if (get_frm(&frmno) == SYSERR) {
        restore(ps);
        return SYSERR;
    }
    frm = &frm_tab[frmno];
    frm->fr_status = FRM_MAPPED;
    frm->fr_pid = pid;
    frm->fr_type = FR_DIR;
    proctab[pid].pdbr = (unsigned long) getaddr_frm(frmno);

    int i, n;
    if (pid == NULLPROC) {
        n = NGPTBL;
    } else {
        n = NPTBL;
    }
    pd_t *pdt = (pd_t *) proctab[pid].pdbr;
    for (i = 0; i < n; i++) {
        if (i < NGPTBL) {
            pdt->pd_pres = 1;
            pdt->pd_base = FRAME0 + 1 + i;		/* location of page table?	*/
        } else {
            pdt->pd_pres = 0;
            pdt->pd_base = 0;
        }
        pdt->pd_write = 1;		/* page is writable?		*/
        pdt->pd_user = 0;		/* is use level protection?	*/
        pdt->pd_pwt	= 0;		/* write through cachine for pt?*/
        pdt->pd_pcd	= 0;		/* cache disable for this pt?	*/
        pdt->pd_acc	= 0;		/* page table was accessed?	*/
        pdt->pd_mbz	= 0;		/* must be zero			*/
        pdt->pd_fmb	= 0;		/* four MB pages?		*/
        pdt->pd_global = 0;		/* global (ignored)		*/
        pdt->pd_avail = 0;		/* for programmer's use		*/
        
        pdt++;
    }
    
    restore(ps);
    return(OK);
}

SYSCALL set_globe_ptbls() {
    STATWORD ps;
    disable(ps);

    fr_map_t *frm;

    int frmno = 0;
    int i;
    for (i = 0; i < NGPTBL; i++) {
        frm = &frm_tab[1 + i];
        frm->fr_status = FRM_MAPPED;
        frm->fr_type = FR_TBL;
        
        int j;
        pt_t *ptt = (pt_t*) getaddr_frm(1 + i);
        for (j = 0; j < NPAGE; j++) {
            ptt->pt_pres = 1;		/* page is present?		*/
            ptt->pt_write = 1;		/* page is writable?		*/
            ptt->pt_user = 0;		/* is use level protection?	*/
            ptt->pt_pwt	= 0;		/* write through for this page? */
            ptt->pt_pcd	= 0;		/* cache disable for this page? */
            ptt->pt_acc	= 0;		/* page was accessed?		*/
            ptt->pt_dirty = 0;		/* page was written?		*/
            ptt->pt_mbz	= 0;		/* must be zero			*/
            ptt->pt_global = 0;		/* should be zero in 586	*/
            ptt->pt_avail = 0;		/* for programmer's use		*/
            ptt->pt_base = frmno++;		/* location of page?		*/

            ptt++;
        }
    }

    restore(ps);
    return(OK);
}

SYSCALL create_pt(int pid, int frmno) {
    STATWORD ps;
    disable(ps);

    if (isbadfrm(frmno) || pid < 0 || pid >= NPROC) {
        restore(ps);
        return SYSERR;
    }

    fr_map_t *frm = &frm_tab[frmno];

    if (frm->fr_status != FRM_UNMAPPED) {
        restore(ps);
        return SYSERR;
    }

    frm->fr_status = FRM_MAPPED;
    frm->fr_type = FR_TBL;
    frm->fr_refcnt = 0;

    int i;
    pt_t *ptt = (pt_t*) getaddr_frm(frmno);
    for (i = 0; i < NPAGE; i++) {
        ptt->pt_pres = 0;		/* page is present?		*/
        ptt->pt_write = 1;		/* page is writable?		*/
        ptt->pt_user = 0;		/* is use level protection?	*/
        ptt->pt_pwt	= 0;		/* write through for this page? */
        ptt->pt_pcd	= 0;		/* cache disable for this page? */
        ptt->pt_acc	= 0;		/* page was accessed?		*/
        ptt->pt_dirty = 0;		/* page was written?		*/
        ptt->pt_mbz	= 0;		/* must be zero			*/
        ptt->pt_global = 0;		/* should be zero in 586	*/
        ptt->pt_avail = 0;		/* for programmer's use		*/
        ptt->pt_base = 0;		/* location of page?		*/

        ptt++;
    }

    restore(ps);
    return OK;
}