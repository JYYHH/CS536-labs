#include "common.h"

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 2 and below network environment:
  - emulates the transmission and delivery (with no loss and no
    corruption) between two physically connected nodes
  - calls the initializations routine rtinit once before
    beginning emulation for each node.

You should read and understand the code below. For Part A, you should fill all parts with annotation starting with "Todo". For Part B and Part C, you need to add additional routines for their features.
******************************************************************/

struct event *evlist = NULL;   /* the event list */
struct distance_table *dts;
int **link_costs; /*This is a 2D matrix stroing the content defined in topo file*/
int num_nodes;
float clocktime = 0.000;


int main(int argc, char *argv[])
{
    struct event *eventptr;
    printevlist();

    /* Todo: Please write the code here to process the input. 
    Given different flag, you have different number of input for part A, B, C. 
    Please write your own code to parse the input for each part. 
    Specifically, in part A you need parse the input file and get “num_nodes”, 
    and fill in the content of dts and link_costs */

    dts = (struct distance_table *) malloc(num_nodes * sizeof(struct distance_table));
    link_costs = (int **) malloc(num_nodes * sizeof(int *));
    for (int i = 0; i < num_nodes; i++)
    {
        link_costs[i] = (int *)malloc(num_nodes * sizeof(int));
    }
    
    for (int i = 0; i < num_nodes; i++)
    {
        rtinit(&dts[i], i, link_costs[i], num_nodes);
    }
    
    
    while (1) 
    {
	/* Todo: Please write the code here to handle the update of time slot k (We assume that in one slot k, the traffic can go through all the routers to reach the destination router)*/

        eventptr = evlist;            /* get next event to simulate */
        if (eventptr==NULL)
           goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist!=NULL)
           evlist->prev=NULL;
        clocktime = eventptr->evtime;    /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER2 ) 
	{
            /* Todo: You need to modify the rtupdate method and add more codes here for Part B and Part C.*/
            rtupdate(&dts[eventptr->eventity], *(eventptr->rtpktptr));
        }
        else 
	{
            printf("Panic: unknown event type\n"); exit(0);
        }
        if (eventptr->evtype == FROM_LAYER2 ) 
          free(eventptr->rtpktptr);        /* free memory for packet, if any */
        free(eventptr);                    /* free memory for event struct   */
    }
   
    

    terminate:
    /* Todo: Please write the code here to handle the case when distance vector converges*/

    return 0;
}
