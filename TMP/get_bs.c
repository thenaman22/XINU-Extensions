#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
    bs_map_t* bptr;
    bptr = &bsm_tab[bs_id];
    if(bptr->bs_status == BSM_UNMAPPED){
        bptr->bs_status = BSM_MAPPED;
        bptr->bs_pid[currpid] = 1;
        bptr->bs_vpno[currpid] = 0;
        bptr->bs_npages[currpid] = npages;
    }
    else{
        if(bptr->bs_heap){
          return SYSERR;
        }
        else if(bptr->bs_npages[currpid]<npages){
            bptr->bs_npages[currpid] = npages;
        }
        bptr->bs_pid[currpid] = 1;
        bptr->bs_vpno[currpid] = 0;
    }
    return npages;
}


