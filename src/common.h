#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* a rtpkt is the packet sent from one router to
   another*/
struct rtpkt{
  int sourceid;       /* id of sending router sending this pkt */
  int destid;         /* id of router to which pkt being sent (must be an directly connected neighbor) */
  int *mincost;    /* min cost to all the node  */
};

struct distance_table{
    int **costs;     // the distance table of curr_node, costs[i][j] is the cost from node i to node j
};

struct event{
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity (node) where event occurs */
   struct rtpkt *rtpktptr; /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
};

/* possible events: */
/*Note in this lab, we only have one event, namely FROM_LAYER2.It refer to that the packet will pop out from layer3, you can add more event to emulate other activity for other layers. Like FROM_LAYER3*/
#define  FROM_LAYER2 1


// define functions here
void insertevent(struct event *p);
void printevlist(); // print the event list
void send2neighbor(struct rtpkt packet);
void build_graph();
/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
void rtinit(struct distance_table *dt, int node, int *link_cost);
void rtupdate(struct distance_table *dt, struct rtpkt recv_pkt);


// define vars here
extern struct event *evlist;
extern struct distance_table *dts;
extern int **link_costs;
extern int num_nodes;
extern int clocktime;
extern FILE *topo_file_path;
extern int kmax;