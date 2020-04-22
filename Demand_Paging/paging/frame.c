/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_tab[NFRAMES];

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  STATWORD ps;
  disable(ps);
  fr_map_t *frm;
  int i;
  for (i = 0; i < NFRAMES; i++) {
    frm = &frm_tab[i];
    frm->fr_status = FRM_UNMAPPED;
    frm->fr_pid = 0;
    frm->fr_vpno = 0;
    frm->fr_refcnt = 0;
    frm->fr_dirty = 0;
    frm->fr_type = FR_PAGE;
  }
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  STATWORD ps;
  disable(ps);
  int i;
  for (i = 0; i < NFRAMES; i++) {
    if (frm_tab[i].fr_status == FRM_UNMAPPED) {
      *avail = i;
      return OK;
    }
  }
  restore(ps);
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int pid, int frmno, int store, int pageth)
{
  STATWORD ps;
  disable(ps);
  if (isbadfrm(frmno) || isbadpid(pid)) {
    restore(ps);
    return SYSERR;
  }
  fr_map_t *frm = &frm_tab[frmno];
  int vpno = frm->fr_vpno;
  unsigned long addr = vpno * NBPG;
  virt_addr_t *vaddr;
  vaddr->pd_offset = (addr >> 22) & 0x3FF;
  vaddr->pt_offset = (addr >> 12) & 0x3FF;
  vaddr->pg_offset = addr & 0xFFF;

  pd_t *pdt = proctab[pid].pdbr + vaddr->pd_offset * sizeof(pd_t);
  pt_t *ptt = pdt->pd_base * NBPG + vaddr->pt_offset * sizeof(pt_t);

  if (frm->fr_status == FRM_MAPPED) {
    if (frm->fr_type == FR_PAGE) {
      if (ptt->pt_dirty == 1) {
        write_bs(getaddr_frm(frmno), store, pageth);
      }
      ptt->pt_pres = 0;
      if (currpid == pid) {
        // invlpg(addr);
      }
      int tblfrmno = pdt->pd_base - FRAME0;
      fr_map_t *tblfrm = &frm_tab[tblfrmno];
      tblfrm->fr_refcnt--;
      if (tblfrm->fr_refcnt == 0 && tblfrmno > 4) { // don't free global tables.
        pdt->pd_pres = 0;
        clear_frm(tblfrmno);
      }
      clear_frm(frmno);
      restore(ps);
      return OK;
    } else if (frm->fr_type == FR_DIR && frm->fr_pid == pid) {
      clear_frm(frmno);
      restore(ps);
      return OK;
    } else if (frm->fr_type == FR_TBL) { // page table is freed automatically.
      restore(ps);
      return SYSERR;
    }
  }
  restore(ps);
  return SYSERR;
}

SYSCALL clear_frm(int i) {
  STATWORD ps;
  disable(ps);
  fr_map_t *frm = &frm_tab[i];
  frm->fr_status = FRM_UNMAPPED;
  frm->fr_pid = 0;
  frm->fr_vpno = 0;
  frm->fr_refcnt = 0;
  frm->fr_dirty = 0;
  frm->fr_type = FR_PAGE;
  restore(ps);
  return OK;
}

SYSCALL lookup_frm(int pid, int vpno, int* frmno) {
  STATWORD ps;
  disable(ps);
  fr_map_t *frm;
  int i;
  for (i = 0; i < NFRAMES; i++) {
    frm = &frm_tab[i];
    if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_PAGE && frm->fr_pid == pid && frm->fr_vpno == vpno) {
      *frmno = i;
      restore(ps);
      return OK;
    }
  }
  restore(ps);
  return SYSERR;
}

char* getaddr_frm(int frmno) {
  return (char*) FRAME_BASE + frmno*FRAME_UNIT_SIZE;
}



