/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  STATWORD ps;
  disable(ps);

  unsigned int addr = read_cr2();

  int vp = addr / NBPG;
  unsigned long pdbr = proctab[currpid].pdbr;

  int store, pageth;
  if (bsm_lookup(currpid, vp, &store, &pageth) == SYSERR) {
    kprintf("0x%x is not a valid virtual address, ", addr);
    kprintf("currpid:%d, vp:%d\n", currpid, vp);
    kill(currpid);
    restore(ps);
    return SYSERR;
  }

  virt_addr_t *vaddr;
  vaddr->pd_offset = (addr >> 22) & 0x3FF;
  vaddr->pt_offset = (addr >> 12) & 0x3FF;
  vaddr->pg_offset = addr & 0xFFF;

  pd_t *pdt = pdbr + vaddr->pd_offset * sizeof(pd_t);
  pt_t *ptt;
  int tblfrmno;
  if (pdt->pd_pres == 0) { // if page table doesn't exist
    // kprintf("Page table doesn't exist\n");
    if (get_frm(&tblfrmno) == SYSERR) {
      restore(ps);
      return SYSERR;
    }
    if (create_pt(currpid, tblfrmno) == SYSERR) {
      restore(ps);
      return SYSERR;
    }
    pdt->pd_pres = 1;
    pdt->pd_base = FRAME0 + tblfrmno;
  } else {
    tblfrmno = pdt->pd_base - FRAME0;
  }
  fr_map_t *tblfrm = &frm_tab[tblfrmno];


  ptt = pdt->pd_base * NBPG + vaddr->pt_offset * sizeof(pt_t);
  int pgfrmno;
  if (ptt->pt_pres == 0) { // if page doesn't exist
    // kprintf("page doesn't exist\n");
    if (get_frm(&pgfrmno) == SYSERR) {
      restore(ps);
      return SYSERR;
    }
    add_frmlist(pgfrmno);
    // print_frmlist();
    fr_map_t *pgfrm = &frm_tab[pgfrmno];
    pgfrm->fr_status = FRM_MAPPED;
    pgfrm->fr_type = FR_PAGE;
    pgfrm->fr_dirty = 0;
    pgfrm->fr_vpno = vp;
    pgfrm->fr_pid = currpid;
    pgfrm->fr_refcnt++;  // increase lifetime counter when allocated 

    ptt->pt_pres = 1;     /* mark page as present */
    ptt->pt_base = FRAME0 + pgfrmno;
    tblfrm->fr_pgcnt++; /* one more page is marked present */

    read_bs(getaddr_frm(pgfrmno), store, pageth);
  }

  restore(ps);
  return OK;
}


