/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int head;
int tail;
int curr_pos;
fr_map_t frm_tab[NFRAMES];
int page_replacement_algo();
int check_set_acc_bit(long , int );
void invalidate_tlb_entry(unsigned long);
void print_frame(int);
/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  int i;
  for(i = 0; i < NFRAMES; i++){
    frm_tab[i].fr_status = FRM_UNMAPPED;
    frm_tab[i].fr_pid = -1;
  }
  head = -1;
  tail = -1;
  curr_pos = -1;
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
  for(i = 0; i<NFRAMES; i++){
    if(frm_tab[i].fr_status == FRM_UNMAPPED){
      *avail = i;
      restore(ps);
      return OK;
    }
  }
  int frame_to_replace = page_replacement_algo();
  print_frame(frame_to_replace);
  unsigned long vp = frm_tab[frame_to_replace].fr_vpno;
  unsigned long a = vp*4096;
  unsigned long pd_offset = vp>>10;
  unsigned long pt_offset = vp & 0x3ff;
  int pid = frm_tab[frame_to_replace].fr_pid;
  
  
  pd_t* pde = (pd_t*)(proctab[pid].pdbr + (sizeof(pd_t)*pd_offset));
  pt_t* pte = (pt_t*) ((pde->pd_base * NBPG) + (sizeof(pt_t)*pt_offset));
  pte->pt_pres = 0;

  // If the page being removed belongs to the current process, invalidate the TLB entry for the page vp using the invlpg instruction (see Intel Volume III/II).
  if(currpid == pid){
    invalidate_tlb_entry((unsigned long)pte);
  }

  fr_map_t* fptr = &frm_tab[pte->pt_base - FRAME0];
  fptr->fr_refcnt--;
  if(fptr->fr_refcnt == 0){
    pde->pd_pres = 0;
    free_frm(i);
  }

  if(pte->pt_dirty == 1){
    int store;
    int pageth;
    if(bsm_lookup(pid, a, &store, &pageth) == SYSERR){
      kill(pid);
      restore(ps);
      return SYSERR;
    }
    write_bs((char*)((FRAME0+frame_to_replace)*NBPG), store, pageth);
  }
  free_frm(frame_to_replace);
  *avail = frame_to_replace;
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
  STATWORD ps;
  disable(ps);

  fr_map_t* fptr;
  fptr = &frm_tab[i];
  fptr->fr_status = FRM_UNMAPPED;
  fptr->fr_pid = -1;
  fptr->fr_vpno = 0;

  restore(ps);
  return OK;
}

int page_replacement_algo(){
  int tmp = curr_pos; 
  while (check_set_acc_bit(frm_tab[tmp].fr_vpno, frm_tab[tmp].fr_pid) == 0) {
      tmp = frm_tab[tmp].next;
  }
  if (tmp == head) {    
    if (head == tail){
      head = -1;
      tail = -1;
    }
    else{
      head = frm_tab[head].next;                 
      frm_tab[head].prev = tail;                 
      frm_tab[tail].next = head;               
    }
  } else if (tmp == tail) {
      tail = frm_tab[tail].prev;                     
      frm_tab[tail].next = head;                     
      frm_tab[head].prev = tail;                     
  } else {
      
      int prev = frm_tab[tmp].prev;
      int next = frm_tab[tmp].next;

      frm_tab[prev].next = next; 
      frm_tab[next].prev = prev; 
  }
  curr_pos = frm_tab[tmp].next;
  return tmp;
}

int check_set_acc_bit(long vpno, int pid){
  unsigned long pd_offset = vpno>>10;
  unsigned long pt_offset = vpno&0x3ff;
  pd_t* pde = (pd_t*)(proctab[pid].pdbr +(sizeof(pd_t)*pd_offset));
  pt_t* pte = (pt_t*)((pde->pd_base*NBPG) + (sizeof(pt_t)*pt_offset));
  if(pte->pt_acc == 0){
    pte->pt_acc = 1;
    return 0;
  }
  else{
    pte->pt_acc = 0;
    return 1;
  }
} 


void invalidate_tlb_entry(unsigned long addr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}

void print_frame(int frame_to_replace){
  if(page_replace_policy == SC)
      kprintf(" Replaced Frame at %d\n", frame_to_replace);
}