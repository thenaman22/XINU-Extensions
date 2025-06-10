#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
  STATWORD ps;
  disable(ps);
  int i;
  bsm_tab[bs_id].bs_status = BSM_UNMAPPED;
  for(i = 0; i < NPROC; i++){
  bsm_tab[bs_id].bs_pid[i] = 0;
  bsm_tab[bs_id].bs_vpno[i] = 0;
  bsm_tab[bs_id].bs_npages[i] = 0;
  }
  
  bsm_tab[bs_id].bs_heap = 0;

  for(i = 0; i<NFRAMES; i++){
    int store, pageth;
    if(bsm_lookup(frm_tab[i].fr_pid, frm_tab[i].fr_vpno*NBPG, &store, &pageth) == SYSERR){
      restore(ps);
      return SYSERR;
    }
    if(store == bs_id){
      write_bs((char*)((FRAME0+i)*NBPG), store, pageth);
      free_frm(i);
    }
  }

  restore(ps);
  return OK;
}