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

// new vars in B
FILE *traffic_file_path;
struct traffic *traf_list;
int traf_len;
// new functions

void output_traffic(){
  printf("k=%d:\n", clocktime);
  for (int i = 0; i < traf_len; i++){
    printf("%d %d %d ", traf_list[i].from, traf_list[i].to, traf_list[i].quant);
    route(traf_list[i].from, traf_list[i].to);
  }
}

int main(int argc, char *argv[]){
  /*
    A.1 handle the input file, and initialize some vars
  */
  if (argc != 4){
    printf("Usage: ./<exe> <k_max(max_iteration_times)> <file_path_to_topo> <file_path_to_traffic>\n");
    exit(1);
  }
  kmax = atoi(argv[1]);
  topo_file_path = fopen(argv[2], "r");
  traffic_file_path = fopen(argv[3], "r");
  traf_list = (struct traffic *)malloc(1000 * sizeof(struct traffic)); // record the list of traffic
  traf_len = 0;
  
  // assume at most 1000 traffic
  build_graph();
    // build traffic
  for(int tr_from, tr_to, tr_quant; fscanf(traffic_file_path, "%d %d %d", &tr_from, &tr_to, &tr_quant) == 3; )
    traf_list[traf_len++] = (struct traffic){tr_from, tr_to, tr_quant};
  
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
      if (clocktime > 0){
        output_traffic();
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
    if (clocktime > 0)
      output_traffic();
    clocktime ++;
  }

  free_work(); // free anything which is not freed
  free(traf_list);

  return 0;
}