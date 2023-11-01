#include "common.h"

// define vars
struct event *evlist = NULL;
struct event *eventptr = NULL;
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
  for (int i = 0; i < num_nodes; i++)
    rtinit(&dts[i], i, link_costs[i]);

  /*
    A.3 DV Update
  */
  while (1){
    // judge whether exit
    eventptr = evlist;
    if (eventptr == NULL) 
      // nothing to transfer
      // but still simulate to round k_max
      break;

    // delete present entry in list
    evlist = evlist->next;
    if (evlist != NULL)
      evlist->prev = NULL;
    
    // judge whether output
    if (clocktime != eventptr->evtime){ // last same slot in past
      if ((clocktime >= 0 && clocktime < 5) || clocktime % 10 == 0){
        output_dvs();
        // output_nxt();
        // printevlist(); // check the events state
      }
    }

    // judge whether exit by clocktime
    clocktime = eventptr->evtime;
    if (clocktime > kmax)
      break;

    // current work
    if (eventptr->evtype == FROM_LAYER2){
      /* Todo: You need to modify the rtupdate method and add more codes here for Part B and Part C.*/
      rtupdate(&dts[eventptr->eventity], eventptr->eventity, eventptr->rtpktptr);
    }
    else{
      printf("Panic: unknown event type\n"); 
      exit(2);
    }

    // free the used event and message object
    if (eventptr->evtype == FROM_LAYER2){
      free(eventptr->rtpktptr->mincost); // extra work to do
      free(eventptr->rtpktptr);
    }
    free(eventptr);
  }
  
  // terminate: convergenced -> simulation
  while (clocktime <= kmax){
    if ((clocktime >= 0 && clocktime < 5) || clocktime % 10 == 0)
      output_dvs();
    clocktime ++;
  }

  free_work(); // free anything which is not freed

  return 0;
}