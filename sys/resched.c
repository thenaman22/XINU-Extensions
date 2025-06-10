/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>
#define lambda 0.1
unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */


	if(getschedclass() == EXPDISTSCHED){
		// Forced Context switch!
		if ((optr = &proctab[currpid])->pstate == PRCURR) {
  			optr->pstate = PRREADY;
  			insert(currpid,rdyhead,optr->pprio);
  		}
		// Generating random Number
		int random_key = (int)expdev(0.1);

		// essentially traversing between rdytail and rdyhead(endpoints excluded)  Queue :: |Rdyhead|0(start)|A|B|C(end)|rdytail|
		int start = q[rdyhead].qnext;
		int end = q[rdytail].qprev;
		int curr = start;

		while (curr != end && random_key >= q[curr].qkey){
			curr = q[curr].qnext;							
		} 
		// Handling NULLPROC : if the queue gets empty(Only NULLPROC) NULLPROC(0) will always be set to run, as it is the first and last (therefore curr(start) == end) process in the queue.
		
		// Removing process from queue and setting it to run.
  		dequeue(curr);
  		nptr = &proctab[ (currpid = curr) ];
  		nptr->pstate = PRCURR;		/* mark it currently running	*/
	}
	else if(getschedclass() == LINUXSCHED){
		// This block of code checks of the currpid process is done running(Yeilds/Sleep) or not. Also decreases the counter as it runs for one more quantum.
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		(optr->counter > 0)) {
			optr->counter = optr->counter - 1;
			return(OK);
		}

		// Suspends the process if its done using its quantum
		if(optr->counter == 0){
			optr->pstate = PRDONE;
			optr->goodness = 0;
		}
			
		// This means that the process has been put to sleep or suspended already!
		else{
			optr->goodness = optr->counter + optr->epoch_prio;
		}

		/* remove highest priority process at end of ready list */
		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		
		/* Checks if epoch ended? If yes updates values for the new epoch*/
		if(currpid == NULLPROC){
			// inserts the nullproc back
			insert(currpid, rdyhead, 0);
			register struct	pentry	*pptr;
			int i;
			for(i = 1; i < NPROC; i++){
				pptr = &proctab[i];
				if(pptr->pstate == PRFREE)
					continue;

				// Sets the epoch Prio-> will only change every epoch
				pptr->epoch_prio = pptr->pprio;

				// Sets the quantum
				if(pptr->counter == pptr->quantum || pptr->counter == 0){   // If did not run then quantum = counter, if created between the epoch then counter = quantum = -1
					pptr->quantum = pptr->epoch_prio;
					pptr->counter = pptr->quantum;
				}
				else{
					pptr->quantum = ((pptr->counter)/2) + pptr->epoch_prio;  // if did not run fully
					pptr->counter = pptr->quantum;
				}
				// sets the goodness!
				pptr->goodness = pptr->counter + pptr->epoch_prio;
				
				// kprintf("%d , %d\n", i , pptr->pstate);
				if(pptr->pstate == PRDONE || pptr->pstate == PRCURR){
					pptr->pstate = PRREADY;
					insert(i,rdyhead, pptr->goodness);
				}
			}
			nptr = &proctab[ (currpid = getlast(rdytail)) ];
			nptr -> pstate = PRCURR;
		}
		else
			nptr->pstate = PRCURR;	/* mark it currently running	*/

		if(currpid == EMPTY){
			xdone();
			shutdown();
		}
		nptr->counter = nptr->counter - 1;
	}
	else{
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
		
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;	/* mark it currently running	*/
	}

	#ifdef	RTCLOCK
    		preempt = QUANTUM;		/* reset preemption counter	*/
    #endif
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}