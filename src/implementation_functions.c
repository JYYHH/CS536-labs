#include "common.h"

void insertevent(struct event *p){
  struct event *q, *qold;

  q = evlist;     /* q points to header of list in which p struct inserted */
  if (q == NULL){   /* list is empty */
    evlist=p;
    p->next=NULL;
    p->prev=NULL;
  }
  else{
    for (qold = q; q !=NULL && p->evtime > q->evtime; q = q->next)
      qold=q; 
    if (q == NULL){   /* end of list */
      qold->next = p;
      p->prev = qold;
      p->next = NULL;
    }
    else if (q == evlist){ /* front of list */
      p->next=evlist;
      p->prev=NULL;
      p->next->prev=p;
      evlist = p;
    }
    else{     /* middle of list */
      p->next=q;
      p->prev=q->prev;
      q->prev->next=p;
      q->prev=p;
    }
  }
}

void printevlist(){
  struct event *q;
  printf("--------------\nEvent List Follows:\n");
  for (q = evlist; q != NULL; q = q->next){
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
  }
  printf("--------------\n");
}


/************************** send update to neighbor (packet.destid)***************/
void send2neighbor(struct rtpkt *packet){
  struct event *evptr;
  int lastime;

 /* be nice: check if  source and destination id's are reasonable */
  if (packet->sourceid < 0 || packet->sourceid > num_nodes){
    printf("WARNING: illegal source id in your packet, ignoring packet!\n");
    return;
  }
  if (packet->destid < 0 || packet->destid > num_nodes){
    printf("WARNING: illegal dest id in your packet, ignoring packet!\n");
    return;
  }
  if (packet->sourceid == packet->destid){
    printf("WARNING: source and destination id's the same, ignoring packet!\n");
    return;
  }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER2;   /* packet will pop out from layer3 */
  evptr->eventity = packet->destid; /* event occurs at other entity */
  evptr->rtpktptr = packet;       /* save ptr to my copy of packet */

/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
  lastime = clocktime;
  evptr->evtime = lastime + 1;
  insertevent(evptr);
} 

void build_graph(){
  int read_buff[105], cnt = 0; // assume that nodes are no more than 10
  while(fscanf(topo_file_path, "%d", read_buff + cnt) == 1)
    cnt ++;
  num_nodes = (int)floor(sqrt(cnt + 0.3));

  link_costs = (int **)malloc(num_nodes * sizeof(int *));
  for (int i = 0; i < num_nodes; i++){
    link_costs[i] = (int *)malloc(num_nodes * sizeof(int));
  }

  for (int i = 0, t = 0; i < num_nodes; i++)
    for (int j = 0; j < num_nodes; j++, t++)
      link_costs[i][j] = read_buff[t];
}

struct rtpkt *build_message(int from, int to, int *msg){
  struct rtpkt *ret = (struct rtpkt *)malloc(sizeof(struct rtpkt));
  ret->sourceid = from;
  ret->destid = to;
  ret->mincost = (int *)malloc(num_nodes * sizeof(int));
  memcpy(ret->mincost, msg, num_nodes * sizeof(int));
  return ret;
}

void rtinit(struct distance_table *dt, int node, int *link_cost){ 
  // use "link_cost" instead of "link_costs" avoiding confusion
  // delete the num_nodes var, cause it's global
  
  dt->costs = (int **)malloc(num_nodes * sizeof(int *));
  for (int i = 0; i < num_nodes; i++){
    dt->costs[i] = (int *)malloc(num_nodes * sizeof(int));
    for (int j = 0; j < num_nodes; j++)
      dt->costs[i][j] = -1; // initially, nothing can be reached
  }

  memcpy(dt->costs[node], link_cost, num_nodes * sizeof(int));

  for (int i = 0; i < num_nodes; i++)
    if (link_cost[i] > 0){ // finds a neighbor
      struct rtpkt *rtp = build_message(node, i, dt->costs[node]);
      send2neighbor(rtp);
    }
}

void recompute_dist(struct distance_table *dt, int node, const int *link_cost){
  // reinitialize
  memcpy(dt->costs[node], link_cost, num_nodes * sizeof(int));

  // using neighbors' information
  for (int i = 0; i < num_nodes; i++)
    if (link_cost[i] > 0) // finds a neighbor
      for (int j = 0; j < num_nodes; j++)
        if (dt->costs[i][j] >= 0){ // a path to 'j'
          int CostALL = link_cost[i] + dt->costs[i][j];
          if (dt->costs[node][j] == -1 || CostALL < dt->costs[node][j])
            dt->costs[node][j] = CostALL;
        }
}

void rtupdate(struct distance_table *dt, int node, struct rtpkt *recv_pkt, const int mode){
  assert(node == recv_pkt->destid); // whether something goes wrong
  const int msg_from = recv_pkt->sourceid;
  const int *link_cost = link_costs[node];

  // 1. copy the original dis_vec
  int *copy_one = (int *)malloc(num_nodes * sizeof(int));
  memcpy(copy_one, dt->costs[node], num_nodes * sizeof(int));

  // 2. update new one
  memcpy(dt->costs[msg_from], recv_pkt->mincost, num_nodes * sizeof(int));

  // 3. recompute 
  recompute_dist(dt, node, link_cost);

  // 4. if change, send msg to neighbors
  if (is_diff(copy_one, dt->costs[node]) || mode == 0)
    for (int i = 0; i < num_nodes; i++)
      if (link_cost[i] > 0){ // finds a neighbor
        struct rtpkt *rtp = build_message(node, i, dt->costs[node]);
        send2neighbor(rtp);
      }
}

void free_work(){
  // free all the message
  while (evlist != NULL){
    if (evlist->evtype == FROM_LAYER2){
      free(evlist->rtpktptr->mincost);
      free(evlist->rtpktptr);
    }
    struct event *old_one = evlist;
    evlist = evlist->next;
    free(old_one);
  }

  // printf("evlist freed\n");

  // free the `link_costs`
  for (int i = 0; i < num_nodes; i++){
    free(link_costs[i]);
  }
  free(link_costs);

  // printf("link_costs freed\n");

  // free the `dts`
  for (int i = 0; i < num_nodes; i++){
    for (int j = 0; j < num_nodes; j++)
      free(dts[i].costs[j]);
    free(dts[i].costs);
  }
  free(dts);

  // printf("dts freed\n");
}

int is_diff(int *cost1, int *cost2){
  for (int i = 0; i < num_nodes; i++)
    if (cost1[i] != cost2[i])
     return 1;
  return 0;
}

void output_dvs(){
  printf("k=%d:\n", clocktime);
  for (int i = 0; i < num_nodes; i++){
    printf("node-%d:", i);
    for (int j = 0; j < num_nodes; j++)
      printf(" %d", dts[i].costs[i][j]);
    puts("");
  }
}