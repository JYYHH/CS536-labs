#include "common.h"

// define vars
struct event *evlist = NULL;
struct distance_table *dts; // for each node
int **link_costs; // But this node knows the global information?
int num_nodes; // how many nodes in network
float clocktime = 0.000; // timestamp
int kmax; // max round of iteration
FILE *topo_file_path;

int main(int argc, char *argv[]){
  /*
    1. handle the input file, and initialize some vars
  */
  struct event *eventptr;
  if (argc != 3){
    printf("Usage: ./<exe> <k_max(max_iteration_times)> <file_path_to_topo>\n");
    exit(1);
  }
  kmax = atoi(argv[1]), topo_file_path = fopen(argv[2], "r");
  build_graph();
  
  for (int i = 0; i < num_nodes; i++){
    rtinit(&dts[i], i, link_costs[i], num_nodes);
  }
  
  
  while (1){
  /* Todo: Please write the code here to handle the update of time slot k (We assume that in one slot k, the traffic can go through all the routers to reach the destination router)*/
    eventptr = evlist;            /* get next event to simulate */
    if (eventptr==NULL)
      goto terminate;
    evlist = evlist->next;        /* remove this event from event list */
    if (evlist!=NULL)
      evlist->prev=NULL;
    clocktime = eventptr->evtime;    /* update time to next event time */
    if (eventptr->evtype == FROM_LAYER2){
      /* Todo: You need to modif y the rtupdate method and add more codes here for Part B and Part C.*/
      rtupdate(&dts[eventptr->eventity], *(eventptr->rtpktptr));
    }
    else{
      printf("Panic: unknown event type\n"); exit(0);
    }
    if (eventptr->evtype == FROM_LAYER2 ) 
      free(eventptr->rtpktptr);        /* free memory for packet, if  any */
    free(eventptr);                    /* free memory for event struct   */
  }
  
  

  terminate:
  /* Todo: Please write the code here to handle the case when distance vector converges*/

  return 0;
}