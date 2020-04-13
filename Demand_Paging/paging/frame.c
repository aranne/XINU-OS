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
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  int i;
  for (i = 0; i < NFRAMES; i++) {
    if (frm_tab[i].fr_status == FRM_UNMAPPED) {
      *avail = i;
      return OK;
    }
  }
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  if (isbadfrm(i)) {
    return SYSERR;
  }
  fr_map_t *frm = &frm_tab[i];
  frm->fr_status = FRM_UNMAPPED;
  frm->fr_pid = 0;
  frm->fr_vpno = 0;
  frm->fr_refcnt = 0;
  frm->fr_dirty = 0;
  frm->fr_type = FR_PAGE;
  return OK;
}



