#ifndef _UNDIRECTED_GRAPH__
#define _UNDIRECTED_GRAPH__
#include "../common/common.h"

#define UG_MAX_VERTEX_NUM 15
#define UG_GEN_EDGE_RATE 3500
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
	UGVertexNode adjmulist[UG_MAX_VERTEX_NUM];
	int vexNum;
	int edgeNum;
}UGraph;

typedef struct UGraphOp{
	UGraph* (*create)();
	void (*destroy)(UGraph**);
	void (*showGraph)(UGraph*);
	void (*DFSTraverse)(UGraph*);
	void (*DFSTraverse_stack)(UGraph*);
	void (*BFSTraverse)(UGraph*);
	void (*addEdge)(UGraph*, int, int, int);
	void (*deleteEdge)(UGraph*, int, int);
	void (*isConnected)(UGraph*);
	bool (*hasCycle)(UGraph*);
}UGraphOp;

extern const UGraphOp* GetUGraphOpStruct();
#define UGraph() (*(GetUGraphOpStruct()))
#endif