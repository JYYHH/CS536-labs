#include "common.h"

// update: Not only recv a dv vector from neighbor will cause the recompute, the change on link will also cause.

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
  // new vars in C
  int time_period;
  int **linkc_[2], now_at = 0;
  /*
    A.1 handle the input file, and initialize some vars
  */
  if (argc != 6){
    printf("Usage: ./<exe> <k_max(max_iteration_times)> <file_path_to_topo_A> <file_path_to_topo_B> <file_path_to_traffic> T_period\n");
    exit(1);
  }
  kmax = atoi(argv[1]);
  traffic_file_path = fopen(argv[4], "r");
  time_period = atoi(argv[5]);
  traf_list = (struct traffic *)malloc(MAX_TRAFFIC * sizeof(struct traffic)); // record the list of traffic
  traf_len = 0;
    // build traffic
  for(int tr_from, tr_to, tr_quant; fscanf(traffic_file_path, "%d %d %d", &tr_from, &tr_to, &tr_quant) == 3; )
    traf_list[traf_len++] = (struct traffic){tr_from, tr_to, tr_quant};
    // build two topologies
  topo_file_path = fopen(argv[2], "r");
  build_graph();
  linkc_[0] = alloc_2d_matrix();
  linkc_[1] = alloc_2d_matrix();
  cp_2d_matrix(linkc_[0], link_costs);
  topo_file_path = fopen(argv[3], "r");
  build_graph();
  cp_2d_matrix(linkc_[1], link_costs);

  /*
    A.2 DV Initialization
  */    
    // initially, the link_costs should be linkc_[0]
  cp_2d_matrix(link_costs, linkc_[now_at]);
  dts = (struct distance_table *)malloc(num_nodes * sizeof(struct distance_table));
  for (int i = 0; i < num_nodes; i++)
    rtinit(&dts[i], i, link_costs[i]);

  /*
    A.3 DV Update
  */
  while (1){
    // judge whether exit
    eventptr = evlist;
    if (eventptr == NULL){
      eventptr = (struct event*)malloc(sizeof(struct event));
      eventptr->evtype = WAIT_EVENT;   /* Converged, but still wait for the change */
      eventptr->evtime = clocktime + 1; // simulate time plus 1
    } 
    else{
      // delete present entry in list, only when it exists
      evlist = evlist->next;
      if (evlist != NULL)
        evlist->prev = NULL;
    }
    
    // judge whether output, or change the topo
    if (clocktime != eventptr->evtime){ // last same slot in past
      if (clocktime > 0){
        output_traffic();
        if (clocktime % time_period == 0){
          now_at ^= 1;
          cp_2d_matrix(link_costs, linkc_[now_at]);
          
          for (int i = 0; i < num_nodes; i++)
            if (is_diff(linkc_[now_at][i], linkc_[now_at^1][i]))
              rtupdate_link_change(&dts[i], i);
        }
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
    else if (eventptr->evtype != WAIT_EVENT){// this is known
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
  
  free_work(); // free anything which is not freed
  free(traf_list);
  free_2d_matrix(linkc_[0]);
  free_2d_matrix(linkc_[1]);

  return 0;
}