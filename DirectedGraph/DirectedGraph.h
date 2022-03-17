#ifndef _DIRECTED_GRAPH__
#define _DIRECTED_GRAPH__
#include "../common/common.h"

#define DG_MAX_VERTEX_NUM 9
#define DG_GEN_EDGE_RATE 0
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
	void (*addEdge)(DGraph*, int, int, int);
	void (*showGraph)(DGraph*);
	bool (*getTopologicalSort)(DGraph*, int*);
	void (*showAllTopologicalSort)(DGraph*);
	void (*showAllCriticalPath)(DGraph*);
}DGraphOp;

extern const DGraphOp* GetDGraphOpStruct();
#define DGraph() (*(GetDGraphOpStruct()))
#endif