#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>


/* a rtpkt is the packet sent from one router to
   another*/
struct rtpkt{
  int sourceid;       /* id of sending router sending this pkt */
  int destid;         /* id of router to which pkt being sent (must be an directly connected neighbor) */
  int *mincost;    /* min cost to all the node  */
};

struct distance_table{
    int **costs;     // the distance table of curr_node, costs[i][j] is the cost from node i to node j
    int *nxts;      // the next node to a specific node
};

struct event{
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity (node) where event occurs */
   struct rtpkt *rtpktptr; /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
};

struct traffic{
   int from;
   int to;
   int quant;
};

/* possible events: */
/*Note in this lab, we only have one event, namely FROM_LAYER2.It refer to that the packet will pop out from layer3, you can add more event to emulate other activity for other layers. Like FROM_LAYER3*/
#define  FROM_LAYER2 1
#define  WAIT_EVENT  2
#define  MAX_TRAFFIC 1000


// define functions here
void insertevent(struct event *p);
void printevlist(); // print the event list
void send2neighbor(struct rtpkt *packet);
void build_graph();
struct rtpkt *build_message(int from, int to, int *msg);
/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/
void rtinit(struct distance_table *dt, int node, int *link_cost);
void recompute_dist(struct distance_table *dt, int node, const int *link_cost);
void rtupdate(struct distance_table *dt, int node, struct rtpkt *recv_pkt);
   // mode = 0 -> for part A, pass the message until k_max reaches, no matter whether DV changes; or other solution (already taken)
   // mode = 1 -> optimal, pass the message only when DV changes
void free_work();
int is_diff(int *cost1, int *cost2);
void output_dvs();
void output_nxt();
void output_traffic();
void route(int src_id, int dst_id);


// define vars here
// extern struct event *eventptr; // no need to use this one as global
extern struct event *evlist;
extern struct distance_table *dts;
extern int **link_costs;
extern int num_nodes;
extern int clocktime;
extern FILE *topo_file_path;
extern int kmax;

// define some fundamental functions
int **alloc_2d_matrix();
void init_1d_vector(int *dst, const int *src);
void init_1d_nxt(int *dst, const int *src);
void cp_2d_matrix(int **dst, int **src);
void free_2d_matrix(int **need_be_fr);