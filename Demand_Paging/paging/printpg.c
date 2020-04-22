#include <conf.h>
#include <kernel.h>
#include <paging.h>

void printtbls() {
    int i;
    fr_map_t *frm;
    for (i = 0; i < NFRAMES; i++) {
        frm = &frm_tab[i];
        if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_TBL) {
            kprintf("TBL--id:%d, refcnt:%d, frm:%d\n", frm->fr_pid, frm->fr_refcnt, i);
        }
    }
}

void printdirs() {
    int i;
    fr_map_t *frm;
    for (i = 0; i < NFRAMES; i++) {
        frm = &frm_tab[i];
        if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_DIR) {
            kprintf("DIR--id:%d, frm:%d\n", frm->fr_pid, i);
        }
    }
}

void printpgs() {
    int i;
    fr_map_t *frm;
    for (i = 0; i < NFRAMES; i++) {
        frm = &frm_tab[i];
        if (frm->fr_status == FRM_MAPPED && frm->fr_type == FR_PAGE) {
            kprintf("PG--id:%d, vp:%d, refcnt:%d, frm:%d\n", frm->fr_pid, frm->fr_vpno, frm->fr_refcnt, i);
        }
    }
}