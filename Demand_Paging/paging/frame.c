/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

fr_map_t frm_tab[NFRAMES];
SYSCALL get_frm_lfu(int *avail);
SYSCALL get_frm_sc(int *avail);

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
    frm->fr_pgcnt = 0;
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
  if (evict_frm(avail) == SYSERR) {
    kprintf("Page Replace Fail(policy:%d).\n", grpolicy());
    restore(ps);
    return SYSERR;
  }

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * evict_frm - replace a frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL evict_frm(int *avail) {
  STATWORD ps;
  disable(ps);

  if (grpolicy() == LFU) {
    if (get_frm_lfu(avail) == SYSERR) {
      restore(ps);
      return SYSERR;
    }
  } else if (grpolicy() == SC) {
    if (get_frm_sc(avail) == SYSERR) {
      restore(ps);
      return SYSERR;
    }
  }

  if (rpdebug) {
    kprintf("\nReplaced Frame Number: %d\n\n", *avail);
  }

  // Swap out frame to backing store
  fr_map_t *frm = &frm_tab[*avail];
  int pid = frm->fr_pid;
  int vp = frm->fr_vpno;
  int store, pageth;
  if (bsm_lookup(pid, vp, &store, &pageth) == SYSERR) {
    kprintf("Replacement failed\n");
    kprintf("Cannot find corresponding BS for frame[%d], ", *avail);
    kprintf("owning pid: %d, vp: %d\n", pid, vp);
    kill(pid);
    restore(ps);
    return SYSERR;
  }
  if (free_frm(pid, *avail, store, pageth) == SYSERR) {
    restore(ps);
    return SYSERR;
  }
  restore(ps);
  return OK;
}

SYSCALL get_frm_lfu(int *avail) {
  STATWORD ps;
  disable(ps);

  int minfrmno = -1;
  int mincnt = MAXINT;
  fr_map_t *frm;
  node* sen = frmlist.sentinel;
  node *p = sen->next;
  while (p != sen) {
    frm = &frm_tab[p->frmno];
    if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_PAGE) {
      if (frm->fr_refcnt < mincnt) {
        mincnt = frm->fr_refcnt;
        minfrmno = p->frmno;
      } else if (frm->fr_refcnt == mincnt) {
        if (frm_tab[minfrmno].fr_vpno < frm->fr_vpno) {
          minfrmno = p->frmno;
        }
      }
    }
    p = p->next;
  }
  if (minfrmno != -1) {
    *avail = minfrmno;
    restore(ps);
    return OK;
  } else {
    restore(ps);
    return SYSERR;
  }
}

SYSCALL get_frm_sc(int *avail) {
  STATWORD ps;
  disable(ps);

  fr_map_t *frm;
  int found = FALSE;
  while (!found && frmlist.size != 0) {
    if (sc_curr->frmno == -1) {
      sc_curr = sc_curr->next;
    }
    frm = &frm_tab[sc_curr->frmno];
    int vp = frm->fr_vpno;
    int pid = frm->fr_pid;
    unsigned long addr = vp * NBPG;
    virt_addr_t *vaddr;
    vaddr->pd_offset = (addr >> 22) & 0x3FF;
    vaddr->pt_offset = (addr >> 12) & 0x3FF;
    vaddr->pg_offset = addr & 0xFFF;
    pd_t *pdt = proctab[pid].pdbr + vaddr->pd_offset * sizeof(pd_t);
    pt_t *ptt = pdt->pd_base * NBPG + vaddr->pt_offset * sizeof(pt_t);
    if (ptt->pt_acc == 1) {
      ptt->pt_acc = 0;
    } else {
      found = TRUE;
      *avail = sc_curr->frmno;
    }
    sc_curr = sc_curr->next;
  }

  if (!found) {
    restore(ps);
    return SYSERR;
  }
  restore(ps);
  return OK;
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
        write_cr3(proctab[pid].pdbr);  // flash TLB entry
      }
      int tblfrmno = pdt->pd_base - FRAME0;
      fr_map_t *tblfrm = &frm_tab[tblfrmno];
      tblfrm->fr_pgcnt--;
      if (tblfrm->fr_pgcnt == 0 && tblfrmno > 4) { // don't free global tables.
        pdt->pd_pres = 0;
        clear_frm(tblfrmno);
      }
      remove_frmlist(frmno);
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

/* DON'T clear fr_refcnt cause it's a lifetime counter */
SYSCALL clear_frm(int i) {
  STATWORD ps;
  disable(ps);
  fr_map_t *frm = &frm_tab[i];
  frm->fr_status = FRM_UNMAPPED;
  frm->fr_pid = 0;
  frm->fr_vpno = 0;
  frm->fr_pgcnt = 0;
  frm->fr_dirty = 0;
  frm->fr_type = FR_PAGE;
  restore(ps);
  return OK;
}

/* find the frame corresponding to certain virtual page */
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



