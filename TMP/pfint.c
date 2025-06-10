/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
pd_t* init_pd(int frame, int pid);
pt_t* init_pt(int* frame, int pid);
/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
  STATWORD ps;
  disable(ps);

  // get the faulted address
  unsigned long faulted_address = read_cr2();
  unsigned long vp = faulted_address>>12;
    // kprintf("Here\n");
  unsigned long pd_offset = (faulted_address>>22) & 0x3ff;
  unsigned long pt_offset = (faulted_address>>12) & 0x3ff;
  unsigned long pg_offset = (faulted_address & 0xfff);

  if (faulted_address < 4096*NBPG){
    restore(ps);
    return SYSERR;
  }


  // if(proctab[currpid].vhpnpages > 0 && faulted_address < (proctab[currpid].vhpno + proctab[currpid].vhpnpages)*NBPG){
  //   restore(ps);
  //   return SYSERR;
  // }
  

  pt_t* page_table;
  int avail_frame;
  fr_map_t* fptr;

  pd_t* corresponding_page_directory_entry = (pd_t*)(proctab[currpid].pdbr + sizeof(pd_t)*(pd_offset));
  if(corresponding_page_directory_entry->pd_pres == 0){
    page_table = init_pt(&avail_frame, currpid);
    corresponding_page_directory_entry->pd_pres = 1;
    corresponding_page_directory_entry->pd_write = 1;
    corresponding_page_directory_entry->pd_base = ((FRAME0+avail_frame));
    fptr = &frm_tab[avail_frame];
  }
  else{
    fptr = &frm_tab[(corresponding_page_directory_entry->pd_base)-FRAME0];
    page_table = (pt_t*)(corresponding_page_directory_entry->pd_base*NBPG);
  }
  int store; 
  int store_offset;
  bsm_lookup(currpid, faulted_address, &store, &store_offset);
  fptr->fr_refcnt++;
  get_frm(&avail_frame);

  if (head == -1) {
      head = avail_frame;
      tail = avail_frame;
      frm_tab[head].next = head; 
      frm_tab[head].prev = head; 
      curr_pos = avail_frame;
      // kprintf("Head got %d\n", head);
  } 
  else {
      frm_tab[avail_frame].next = frm_tab[tail].next;
      frm_tab[avail_frame].prev = tail;               
      frm_tab[frm_tab[tail].next].prev = avail_frame; 
      frm_tab[tail].next = avail_frame;               
      tail = avail_frame; 
      // kprintf("Head got %d Tail got %d\n", head, tail);
  }

  read_bs((char*)((FRAME0+avail_frame)*NBPG), store, store_offset);
    {    
        frm_tab[avail_frame].fr_status = FRM_MAPPED;
        frm_tab[avail_frame].fr_pid = currpid;
        frm_tab[avail_frame].fr_type = FR_PAGE;
        frm_tab[avail_frame].fr_vpno = (vp);
        frm_tab[avail_frame].fr_dirty = 0;
    }
  pt_t* corresponding_page_table_entry = (pt_t*)(((corresponding_page_directory_entry->pd_base)*NBPG) + (sizeof(pt_t)*(pt_offset)));
  corresponding_page_table_entry->pt_pres = 1;
  corresponding_page_table_entry->pt_base = ((FRAME0+avail_frame));

  restore(ps);
  return OK;
}

pt_t* init_pt(int *frame, int pid){
  if( get_frm(frame) == SYSERR)
    return SYSERR;
  frm_tab[*frame].fr_type = FR_TBL;
  frm_tab[*frame].fr_status = FRM_MAPPED;
  frm_tab[*frame].fr_pid = pid;

  int i;
  pt_t *pte;

  pte = (pt_t *)((FRAME0+(*frame))*NBPG);
  for(i=0; i < 1024; i++){
    pte->pt_pres = 0;
    pte->pt_write = 1;
    pte->pt_user = 0;
    pte->pt_pwt = 0;
    pte->pt_pcd = 0;
    pte->pt_acc = 0;
    pte->pt_dirty = 0;
    pte->pt_mbz = 0;
    pte->pt_global = 0;
    pte->pt_avail = 0;
    pte++; 		
  }
  return (pt_t*)((FRAME0+(*frame))*NBPG);
}
pd_t* init_pd(int frame, int pid){
  pd_t* page_directory = (pd_t*)((FRAME0+frame)*NBPG);
  pd_t* tmp = page_directory;
  fr_map_t* fptr;
  fptr = &frm_tab[frame];
  fptr->fr_status = FRM_MAPPED;
  fptr->fr_pid = pid;
  fptr->fr_type = FR_DIR;
  int i;
  for(i = 0; i< 1024; i++){
    if(i<4){
      tmp->pd_pres= 1;
      tmp->pd_base= ((FRAME0+i));
    }
    else{
      tmp->pd_pres= 0;
      tmp->pd_base= 0;
    }
    tmp->pd_write = 1;
    tmp->pd_user=0;
    tmp->pd_pwt=0;
    tmp->pd_pcd=0;
    tmp->pd_acc=0;
    tmp->pd_mbz=0;
    tmp->pd_fmb=0;
    tmp->pd_global=0;
    tmp->pd_avail=0;
    tmp++;
  }
  return (pd_t*)((FRAME0+frame)*NBPG);
}


