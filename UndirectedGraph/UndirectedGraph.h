#ifndef _UNDIRECTED_GRAPH__
#define _UNDIRECTED_GRAPH__
#include "../common/common.h"

#define UG_MAX_VERTEX_NUM 10
#define UG_GEN_EDGE_RATE 5000
#define UG_MAX_WEIGHT 100

typedef struct UGEdgeNode{
	struct UGEdgeNode* ilink;
	struct UGEdgeNode* jlink;
	int ivex;
	int jvex;
	int weight;
}UGEdgeNode;

typedef struct UGVertexNode{
	int data;
	UGEdgeNode* firstEdge;
}UGVertexNode;

typedef struct UGraph{
	UGVertexNode adjmulist[MAX_VERTEX_NUM];
	int vexNum;
	int edgeNum;
}UGraph;

#endif