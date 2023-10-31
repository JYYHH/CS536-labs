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
    for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
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
  for (q = evlist; q!=NULL; q=q->next){
    printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
  }
  printf("--------------\n");
}


/************************** send update to neighbor (packet.destid)***************/
void send2neighbor(struct rtpkt packet){
  struct event *evptr;
  int lastime;

 /* be nice: check if  source and destination id's are reasonable */
  if (packet.sourceid < 0 || packet.sourceid > num_nodes){
    printf("WARNING: illegal source id in your packet, ignoring packet!\n");
    return;
  }
  if (packet.destid < 0 || packet.destid > num_nodes){
    printf("WARNING: illegal dest id in your packet, ignoring packet!\n");
    return;
  }
  if (packet.sourceid == packet.destid){
    printf("WARNING: source and destination id's the same, ignoring packet!\n");
    return;
  }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER2;   /* packet will pop out from layer3 */
  evptr->eventity = packet.destid; /* event occurs at other entity */
  evptr->rtpktptr = &packet;       /* save ptr to my copy of packet */

/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
  lastime = clocktime;
  evptr->evtime = lastime + 1;
  insertevent(evptr);
} 

void build_graph(){
  int read_buff[105], cnt = 0;
  while(fscanf(topo_file_path, "%d", read_buff + cnt) == 1)
    cnt ++;
  num_nodes = (int)floor(sqrt(cnt + 0.3));
  dts = (struct distance_table *) malloc(num_nodes * sizeof(struct distance_table));
  link_costs = (int **) malloc(num_nodes * sizeof(int *));
  for (int i = 0; i < num_nodes; i++){
    link_costs[i] = (int *)malloc(num_nodes * sizeof(int));
  }

  for (int i = 0, t = 0; i < num_nodes; i++)
    for (int j = 0; j < num_nodes; j++, t++)
      link_costs[i][j] = read_buff[t];
}

void rtinit(struct distance_table *dt, int node, int *link_costs, int num_nodes)
{
    /* Todo: Please write the code here*/




}

void rtupdate(struct distance_table *dt, struct rtpkt recv_pkt)
{
    /* Todo: Please write the code here*/

}