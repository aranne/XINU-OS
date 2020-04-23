/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;   /* replacement policy for paging */
void setrpdebug(void);

/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  STATWORD ps;
  disable(ps);

  if (policy != SC && policy != LFU) {
    kprintf("Wrong number of replacement policy!\n");
    restore(ps);
    return SYSERR;
  }

  page_replace_policy = policy;
  setrpdebug();             // ture on debug mode

  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
void setrpdebug() {
  rpdebug = TRUE;
}
