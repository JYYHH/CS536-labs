#include "common.h"

// define vars
struct event *evlist = NULL;
struct distance_table *dts; // for each node
int **link_costs; // >0 <-> neighbor
int num_nodes; // how many nodes in network
int clocktime = 0; // timestamp
int kmax; // max round of iteration
FILE *topo_file_path;

int main(int argc, char *argv[]){
  /*
    A.1 handle the input file, and initialize some vars
  */
  struct event *eventptr;
  if (argc != 3){
    printf("Usage: ./<exe> <k_max(max_iteration_times)> <file_path_to_topo>\n");
    exit(1);
  }
  kmax = atoi(argv[1]), topo_file_path = fopen(argv[2], "r");
  build_graph();
  
  /*
    A.2 DV Initialization
  */
  dts = (struct distance_table *)malloc(num_nodes * sizeof(struct distance_table));
  for (int i = 0; i < num_nodes; i++){
    rtinit(&dts[i], i, link_costs[i]);
  }
  
  /*
    A.3 DV Update
  */

  while (1){
    eventptr = evlist;
    if (eventptr == NULL || eventptr->evtime > kmax) // max_iteration
      break;
    evlist = evlist->next;
    if (evlist!=NULL)
      evlist->prev = NULL;
    clocktime = eventptr->evtime;    /* update time to next event time */
    if (eventptr->evtype == FROM_LAYER2){
      /* Todo: You need to modify the rtupdate method and add more codes here for Part B and Part C.*/
      rtupdate(&dts[eventptr->eventity], *(eventptr->rtpktptr));
    }
    else{
      printf("Panic: unknown event type\n"); exit(0);
    }
    if (eventptr->evtype == FROM_LAYER2 ) 
      free(eventptr->rtpktptr);        /* free memory for packet, if  any */
    free(eventptr);                    /* free memory for event struct   */
  }
  
  // terminate: convergence


  return 0;
}