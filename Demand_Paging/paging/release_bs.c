#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
  if (isbadbs(bs_id))
    return SYSERR;
  bs_map_t *bsm = &bsm_tab[bs_id];
  if (bsm->bs_status == BSM_UNMAPPED) {
    free_bsm(bs_id);
    return OK;
  } else if (bsm->bs_status == BSM_SHARED) {
    if (bsm->bs_vp->bs_pid == currpid && bsm->bs_vp->nextvp->bs_pid == -1) {
      free_bsm(bs_id);
      return OK;
    } else {
      return SYSERR;
    }
  } else {
    if (bsm->bs_vp->bs_pid == currpid) {
      free_bsm(bs_id);
      return OK;
    } else {
      return SYSERR;
    }
  }
}

