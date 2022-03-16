#include "DirectedGraph.h"
#include "../SqStack/SqStack.h"
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
		for (int i = 0; i < j; i++){
			if (rand() % 10000 > DG_GEN_EDGE_RATE * (i + 1) / (j + 1))
				continue;
			POINTCREATE_INIT(DGEdgeNode*, enode, DGEdgeNode, sizeof(DGEdgeNode));
			enode->weight = rand() % DG_MAX_WEIGHT + 1;
			if (rand() % 2 == 0){
				enode->tailvex = i;
				enode->headvex = j;
			}
			else{
				enode->headvex = i;
				enode->tailvex = j;
			}
			DGEdgeNode* edge = g->orthlist[enode->headvex].firstin;
			if (edge){
				while(edge->hlink)
					edge = edge->hlink;
				edge->hlink = enode;
			}
			else
				g->orthlist[enode->headvex].firstin = enode;
			edge = g->orthlist[enode->tailvex].firstout;
			if (edge){
				while(edge->tlink)
					edge = edge->tlink;
				edge->tlink = enode;
			}
			else
				g->orthlist[enode->tailvex].firstout = enode;
			edgeNum++;
			printf("gen--<v%-2d v%-2d %3d>\n", enode->tailvex, enode->headvex, enode->weight);
		}
	}
	g->vexNum = DG_MAX_VERTEX_NUM;
	g->edgeNum = edgeNum;

	return g;
}

static void destroy(DGraph** sgraph)
{
	for (int i = 0; i < (*sgraph)->vexNum; i++){
		DGEdgeNode* edge = (*sgraph)->orthlist[i].firstin;
		while(edge){
			DGEdgeNode* tmp = edge;
			edge = edge->hlink;
			FREE(tmp);
		}
	}
	FREE(*sgraph);
}

static void showGraph(DGraph* graph)
{
	if (!graph->vexNum){
		printf("empty graph.\n");
		return;
	}
	if (graph->vexNum == 1)
		printf("vertex:0\nv0\n");
	else
		printf("vertex:%d\nv0~v%d\n", graph->vexNum, graph->vexNum - 1);

	if (!graph->edgeNum){
		printf("edge:0\n");
		return;
	}
	printf("edge:%d\n", graph->edgeNum);
	int count = 0;
	for (int i = 0; i < graph->vexNum; i++){
		DGEdgeNode* edge = graph->orthlist[i].firstin;
		while(edge){
			printf("<%-2d %-2d %3d>\t", edge->tailvex, edge->headvex, edge->weight);
			edge = edge->hlink;
			count++;
		}
	}
	puts("");
	printf("total edge output:%d\n", count);
}

static void showTopologicalSort(DGraph* graph)
{
	int indegree[DG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++){
		int degree = 0;
		DGEdgeNode* edge = graph->orthlist[i].firstin;
		while(edge){
			degree++;
			edge = edge->hlink;
		}
		indegree[i] = degree;
	}
	int topoList[DG_MAX_VERTEX_NUM];
	int topoIdx = 0;
	for (int i = 0; i < graph->vexNum; i++){
		if (indegree[i])
			continue;
		topoList[topoIdx++] = i;
		indegree[i]--;
		DGEdgeNode* edge = graph->orthlist[i].firstout;
		while(edge){
			indegree[edge->headvex]--;
			edge = edge->tlink;
		}
		i = -1;
	}
	if (topoIdx != graph->vexNum){
		printf("this graph is cycled.\n");
		return;
	}
	puts("topological order:");
	for (int i = 0; i < topoIdx - 1; i++)
		printf("%d, ", topoList[i]);
	printf("%d", topoList[topoIdx - 1]);
	puts("");
}

static void showAllTopologicalSort(DGraph* graph)
{
	int indegree[DG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++){
		int degree = 0;
		DGEdgeNode* edge = graph->orthlist[i].firstin;
		while(edge){
			degree++;
			edge = edge->hlink;
		}
		indegree[i] = degree;
	}
	int topoList[DG_MAX_VERTEX_NUM];
	SqStack* trace_stack[DG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++)
		trace_stack[i] = SqStack().create(sizeof(int), NULL);
	int trace_idx = 0;
	while(true){
		while(true){
			for (int i = 0; i < graph->vexNum; i++){
				if (indegree[i])
					continue;
				SqStack().push(trace_stack[trace_idx], &i);
			}
			if (SqStack().empty(trace_stack[trace_idx])){
				printf("this graph is cycled.%d\n", trace_idx);
				for (int i = 0; i < graph->vexNum; i++)
					printf("%d, ", indegree[i]);
				puts("");
				for (int i = 0; i < graph->vexNum; i++)
					SqStack().destroy(trace_stack + i);
				return;
			}
			topoList[trace_idx] = TOCONSTANT(int, SqStack().pop(trace_stack[trace_idx]));
			indegree[topoList[trace_idx]]--;
			DGEdgeNode* edge = graph->orthlist[topoList[trace_idx]].firstout;
			while(edge){
				indegree[edge->headvex]--;
				edge = edge->tlink;
			}
			if (++trace_idx == graph->vexNum)
				break;
		}
		trace_idx--;
		for (int i = 0; i < trace_idx; i++)
			printf("%d, ", topoList[i]);
		printf("%d\n", topoList[trace_idx]);
		while(SqStack().empty(trace_stack[trace_idx])){
			if (trace_idx == 0)
				break;
			indegree[topoList[trace_idx]]++;
			DGEdgeNode* edge = graph->orthlist[topoList[--trace_idx]].firstout;
			while(edge){
				indegree[edge->headvex]++;
				edge = edge->tlink;
			}
		}
		if (trace_idx == 0 && SqStack().empty(trace_stack[0]))
			break;
		indegree[topoList[trace_idx]]++;
		topoList[trace_idx] = TOCONSTANT(int, SqStack().pop(trace_stack[trace_idx]));
		indegree[topoList[trace_idx]]--;
		trace_idx++;
	}
	for (int i = 0; i < graph->vexNum; i++)
		SqStack().destroy(trace_stack + i);
}

inline const DGraphOp* GetDGraphOpStruct()
{
	static const DGraphOp OpList = {
		.create = create,
		.destroy = destroy,
		.showGraph = showGraph,
		.showTopologicalSort = showTopologicalSort,
		.showAllTopologicalSort = showAllTopologicalSort,
	};
	return &OpList;
}