/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0; 
            }
        }
	initialized = 1;
    }
    
    /* TODO: Implement LRU Paging */
    for(proctmp=0; proctmp< MAXPROCESSES; proctmp++){
        // Updates all of the timestamps on all of the loaded in pages so we know 
        // which one to remove as time goes on.  
        if(q[proctmp].active){
            // Note the the PC counter is minus 1 here, this is because pc/PAGESIZE is the incoming page
            int currentPage = q[proctmp].pc-1/PAGESIZE;
            timestamps[proctmp][currentPage]++;
        }
    }

    // unfortunatly we have to repeat the same look here so that each
    // process has a chance to be updated on the timestamp
    for(proctmp=0; proctmp< MAXPROCESSES; proctmp++){

        int nextpage = q[proctmp].pc/MAXPROCPAGES;
        //We need to remove all of the finished pages first so we can create as much space as we can 
        // before implenting LRU
        if (q[proctmp].active == 0){
            for(pagetmp = 0; pagetmp < MAXPROCPAGES; pagetmp++){
                pageout(proctmp, pagetmp); //takes all pages of a finished process out
            }
        }

        //For the current page being accesses (nextpage) we need to first see if its even loaded
        if(q[proctmp].pages[nextpage] == 1){ // a 1 here means that its either: swapped/swapping, or can be swapped in
            break; // if it is, then no more work needs to be done and we can leave
        }

        //If its swapped out, we can try and swap it in (hoping there is some open space)
        if(pagein(proctmp, nextpage)){ //if this returns 1 then the page is either: swapping in or can be swapped in
            q[proctmp].pages[pagetmp] = 1; // update the page bitmap as loaded/loading in.
            break; // no more needs to be done.
        }

        //Before we boot something, we need to make sure that this page isn't currently being swapped out
        if(q[proctmp].pages[pagetmp] == 0)
            break; //noting we can do but wait and try again in another cycle

        //If we get here then that means there was no space left in the memory and we need to 
        //boot something. Using a bubble sort logic, we can just keep a track of the page with
        //the longest timestamp. Observe:
        int TmpProcTmp, pagetmptmp;
        int pageWithLongestTimestamp = 0;
        int procWithLongestTimestamp = 0;
        for(TmpProcTmp = 0; TmpProcTmp < MAXPROCESSES; TmpProcTmp++){
            for(pagetmptmp = 0; pagetmptmp < MAXPROCPAGES; pagetmptmp++){
                if(timestamps[TmpProcTmp][pagetmptmp] > pageWithLongestTimestamp){
                    pageWithLongestTimestamp = pagetmptmp;
                    procWithLongestTimestamp = TmpProcTmp;
                }
            }
        }
        pageout(procWithLongestTimestamp, pageWithLongestTimestamp);
        q[procWithLongestTimestamp].pages[pageWithLongestTimestamp] = 0;//signles proc is shifting out
        q[proctmp].pages[nextpage] = 1; //signal new page is available to swap in.
    }

    /* advance time for next pageit iteration */
    tick++;
} 

///////////////////////////////////// Planning (for personal purpose) ///////////////////////////////////
/*
What needs to happen:

Overview:
pages which are finished need to be swapped out
Each page currently loaded needs to have its timestamp updated so its duration in main memory is recorded.

We need to update the bit map for which pages are currently swapped in or swapped out (or in the process of either)
The oldest page needs to be identified and swapped out
The new page needs to be swapped in........
*/
