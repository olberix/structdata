#include "DirectedGraph.h"
#include "../SkipList/SkipList.h"
#include "../DlQueue/DlQueue.h"
#include "../SqStack/SqStack.h"
#include "../PriorityQueue/PriorityQueue.h"
#include <time.h>
#include <limits.h>

//这里有向图侧重实现拓扑排序和关键路径,所以生成随机有向图的时候不生成环
static DGraph* create()
{
	POINTCREATE_INIT(DGraph*, g, DGraph, sizeof(DGraph));
	int edgeNum = 0;
	for (int j = 0; j < DG_MAX_VERTEX_NUM; j++){
		DGVertexNode vex;
		vex.data = j;
		vex.firstin = vex.firstout = NULL;

		g->orthlist[j] = vex;
	}
	g->vexNum = DG_MAX_VERTEX_NUM;
	g->edgeNum = edgeNum;

	return g;
}

static void __destroy(DGraph* graph, DGEdgeNode* edge, SkipList* skl)
{

}

static void destroy(DGraph** sgraph)
{

}

static void showGraph(DGraph* graph)
{

}

inline const DGraphOp* GetDGraphOpStruct()
{
	static const DGraphOp OpList = {
		.create = create,
		.destroy = destroy,
		.showGraph = showGraph,
	};
	return &OpList;
}