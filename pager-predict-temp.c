/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <limits.h>

#include "simulator.h"

struct pagedata {
    int page;
    int count;
    int *timestamp;
};
typedef struct pagedata PageData;

void addData(int currentpage, int proc, int lastpage,
        PageData counts[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES],
            int timestamps[MAXPROCESSES][MAXPROCPAGES]){
    int i;
    PageData *tempData;
    tempData = counts[proc][lastpage];

    for(i = 0; i < MAXPROCPAGES; i++){
        if(tempData[i].page == currentpage){
            tempData[i].count += 1;
            break;
        }
        else if(tempData[i].page == -1) {
            tempData[i].page = currentpage;
            tempData[i].count = 1;
            tempData[i].timestamp = &(timestamps[proc][i]);
            break;
        }
    }

}

int lru(Pentry q[MAXPROCESSES], int proc, int* pagelru, int timestamps[MAXPROCESSES][MAXPROCPAGES]){
    int i, timestamp; 
    int minTimestamp = INT_MAX;
    int ret = 0;

    for(i = 0; i < q[proc].npages; i++) {
        timestamp = timestamps[proc][i];
        if((timestamp < minTimestamp) && q[proc].pages[i]){
            minTimestamp = timestamp;
            *pagelru = i;
            ret = 1;
        }
    }
    return ret;
}

int getLen(PageData* predictions){
    int len = 0;
    while(predictions[len].page != -1 && len<MAXPROCPAGES){
        len++;
    }
    return len;
}


void sortData(PageData* predictions){
    int len = getLen(predictions);
    int i;
    int run = 1;
    while(run){
        run = 0;
        for(i = 1; i < len; i++){
            if(predictions[i - 1].timestamp != NULL && predictions[i].timestamp != NULL){
                if(predictions[i - 1].count < predictions[i].count){
                // if(*(predictions[i - 1].timestamp) < *(predictions[i].timestamp)){
                    PageData temp = predictions[i - 1];
                    predictions[i - 1] = predictions[i];
                    predictions[i] = temp;
                    run = 1;
                }
            }
        }
    }

}

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for a predictive pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];
    static int lastpc[MAXPROCESSES];
    static PageData counts[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];
    
    /* Local vars */
    int proctmp, pagetmp, nextpagetmp, proc, page, lastpage;
    

    /* initialize static vars on first run */
    if(!initialized){
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
                for(nextpagetmp=0; nextpagetmp < MAXPROCPAGES; nextpagetmp++){
                    counts[proctmp][pagetmp][nextpagetmp].page = -1;
                    counts[proctmp][pagetmp][nextpagetmp].count = -1;
                    counts[proctmp][pagetmp][nextpagetmp].timestamp = NULL;
                }
            }
        }
        initialized = 1;
    }

    /* TODO: Implement Predictive Paging */
    for(proc = 0; proc < MAXPROCESSES; proc++){
        // if process is active
        if(q[proc].active) {
            // test if last page is same as this page (if not pageout last page)
            page = q[proc].pc/PAGESIZE;
            lastpage = lastpc[proc]/PAGESIZE;
            lastpc[proc] = q[proc].pc;
            if(lastpage != page){
                pageout(proc, lastpage);
                addData(page, proc, lastpage, counts, timestamps);
            }

            // update timestamps
            pagetmp = (q[proc].pc - 1)/PAGESIZE;
            timestamps[proc][pagetmp] = tick;

            // if page isn't in memory
            if(!q[proc].pages[page]){
                // if page in didn't work (aka no available memory)
                if(!pagein(proc, page)){
                    // find LRU (if successful remove LRU)
                    if(lru(q, proc, &pagetmp, timestamps)){
                        if(!pageout(proc, pagetmp)){
                            fprintf(stderr, "Error: Page out failed (LRU)");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
        // pageout if inactive 
        else {
            for(int i = 0; i < q[proc].npages; i++){
                if(!pageout(proc, i)){
                    fprintf(stderr, "Error: Page out failed (Inactive)");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    // make predictions
    for(proc = 0; proc < MAXPROCESSES; proc++){
        PageData* predictions;
        int i;
        // get all active process future pages and sort by EITHER timestamp OR count
        if(q[proc].active){
            predictions = counts[proc][(q[proc].pc + 101)/PAGESIZE];
            sortData(predictions);
        }

        int len = getLen(predictions);
        for(i = 0; i < len; i++){
            pagein(proc, predictions[i].page);
        }
    }

    /* advance time for next pageit iteration */
    tick++;
} 
