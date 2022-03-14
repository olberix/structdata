#ifndef _DIRECTED_GRAPH__
#define _DIRECTED_GRAPH__
#include "../common/common.h"

#define DG_MAX_VERTEX_NUM 20
#define DG_GEN_EDGE_RATE 3500
#define DG_MAX_WEIGHT 100

typedef struct DGEdgeNode{
	int headvex, tailvex;
	int weight;
	struct DGEdgeNode *hlink, *tlink;
}DGEdgeNode;

typedef struct DGVertexNode{
	int data;
	DGEdgeNode *firstin, *firstout;
}DGVertexNode;

typedef struct DGraph{
	DGVertexNode orthlist[DG_MAX_VERTEX_NUM];
	int vexNum, edgeNum;
}DGraph;

typedef struct DGraphOp{
	DGraph* (*create)();
	void (*destroy)(DGraph**);
	void (*showGraph)(DGraph*);
}DGraphOp;

extern const DGraphOp* GetDGraphOpStruct();
#define DGraph() (*(GetDGraphOpStruct()))
#endif