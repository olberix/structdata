#include "DirectedGraph.h"
#include "../SqStack/SqStack.h"
#include <time.h>
#include <limits.h>

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
			if (rand() % 10000 + 1 > DG_GEN_EDGE_RATE * (i + 1) / (j + 1))
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

static void addEdge(DGraph* graph, int tailvex, int headvex, int weight)
{
	DGEdgeNode* edge = graph->orthlist[tailvex].firstout;
	while(edge){
		if (edge->headvex == headvex)
			return;
		if (edge->tlink)
			edge = edge->tlink;
		else
			break;
	}
	POINTCREATE_INIT(DGEdgeNode*, enode, DGEdgeNode, sizeof(DGEdgeNode));
	enode->headvex = headvex;
	enode->tailvex = tailvex;
	enode->weight = weight;
	if (edge)
		edge->tlink = enode;
	else
		graph->orthlist[tailvex].firstout = enode;

	edge = graph->orthlist[headvex].firstin;
	if (edge){
		while(edge->hlink)
			edge = edge->hlink;
		edge->hlink = enode;
	}
	else
		graph->orthlist[headvex].firstin = enode;
	graph->edgeNum++;
}

static void showGraph(DGraph* graph)
{
	if (!graph->vexNum){
		printf("empty graph.\n");
		return;
	}
	if (graph->vexNum == 1)
		printf("vertex:1\nv0\n");
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
			printf("<%-2d %-2d %2d>\t", edge->tailvex, edge->headvex, edge->weight);
			edge = edge->hlink;
			count++;
		}
	}
	puts("");
	printf("total edge output:%d\n", count);
}

static bool getTopologicalSort(DGraph* graph, int* topoList)
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
	if (topoIdx != graph->vexNum)
		return false;
	return true;
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
	int trace_idx = 0, count = 0;
	while(true){
		do{
			for (int i = 0; i < graph->vexNum; i++){
				if (indegree[i])
					continue;
				SqStack().push(trace_stack[trace_idx], &i);
			}
			if (SqStack().empty(trace_stack[trace_idx])){
				printf("this graph is cycled.\n");
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
		}while(++trace_idx != graph->vexNum);
		trace_idx--;
		for (int i = 0; i < trace_idx; i++)
			printf("%d, ", topoList[i]);
		printf("%d\n", topoList[trace_idx]);
		count++;
		do{
			indegree[topoList[trace_idx]]++;
			DGEdgeNode* edge = graph->orthlist[topoList[trace_idx]].firstout;
			while(edge){
				indegree[edge->headvex]++;
				edge = edge->tlink;
			}
		}while(SqStack().empty(trace_stack[trace_idx]) && --trace_idx >= 0);
		if (trace_idx < 0)
			break;
		topoList[trace_idx] = TOCONSTANT(int, SqStack().pop(trace_stack[trace_idx]));
		indegree[topoList[trace_idx]]--;
		DGEdgeNode* edge = graph->orthlist[topoList[trace_idx]].firstout;
		while(edge){
			indegree[edge->headvex]--;
			edge = edge->tlink;
		}
		trace_idx++;
	}
	printf("total topological order output:%d\n", count);
	for (int i = 0; i < graph->vexNum; i++)
		SqStack().destroy(trace_stack + i);
}

static void showAllCriticalPath(DGraph* graph)
{
	//判断源点汇点合法性
	int source_count = 0, target_count = 0;
	for (int i = 0; i < graph->vexNum; i++){
		if (!graph->orthlist[i].firstin)
			source_count++;
		if (!graph->orthlist[i].firstout)
			target_count++;
	}
	if (source_count != 1 || target_count != 1){
		puts("invalid AOE-net.");
		return;
	}
	int topoList[DG_MAX_VERTEX_NUM];
	if (!getTopologicalSort(graph, topoList)){
		puts("this graph is cycled.");
		return;
	}
	int ETV[DG_MAX_VERTEX_NUM];
	memset(ETV, 0, sizeof(ETV));
	for (int i = 0; i < graph->vexNum; i++){
		DGEdgeNode* edge = graph->orthlist[topoList[i]].firstout;
		while(edge){
			if (ETV[edge->headvex] < ETV[topoList[i]] + edge->weight)
				ETV[edge->headvex] = ETV[topoList[i]] + edge->weight;
			edge = edge->tlink;
		}
	}
	int LTV[DG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++)
		LTV[i] = ETV[graph->vexNum - 1];
	for (int i = graph->vexNum - 1; i >= 0; i--){
		DGEdgeNode* edge = graph->orthlist[topoList[i]].firstout;
		while(edge){
			if (LTV[topoList[i]] > LTV[topoList[edge->headvex]] - edge->weight)
				LTV[topoList[i]] = LTV[topoList[edge->headvex]] - edge->weight;
			edge = edge->tlink;
		}
	}
	SqStack* stack_vex[DG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++)
		stack_vex[i] = SqStack().create(sizeof(int), NULL);
	int critical_path[DG_MAX_VERTEX_NUM];
	int idx = 0, count = 0;
	critical_path[idx++] = topoList[0];
	while (true){
		do{
			DGEdgeNode* edge = graph->orthlist[critical_path[idx - 1]].firstout;
			while(edge){
				if (ETV[edge->headvex] == LTV[edge->headvex])
					SqStack().push(stack_vex[idx], &(edge->headvex));
				edge = edge->tlink;
			}
			critical_path[idx] = TOCONSTANT(int, SqStack().pop(stack_vex[idx]));
		}while(critical_path[idx++] != topoList[graph->vexNum - 1]);
		idx--;
		for (int i = 0; i < idx; i++)
			printf("%d, ", critical_path[i]);
		printf("%d\n", critical_path[idx]);
		count++;
		while(SqStack().empty(stack_vex[idx]) && --idx >= 0){}
		if (idx < 0)
			break;
		critical_path[idx] = TOCONSTANT(int, SqStack().pop(stack_vex[idx]));
		idx++;
	}
	printf("total critical path output:%d\n", count);
	for (int i = 0; i < graph->vexNum; i++)
		SqStack().destroy(stack_vex + i);
}

inline const DGraphOp* GetDGraphOpStruct()
{
	static const DGraphOp OpList = {
		.create = create,
		.destroy = destroy,
		.addEdge = addEdge,
		.showGraph = showGraph,
		.getTopologicalSort = getTopologicalSort,
		.showAllTopologicalSort = showAllTopologicalSort,
		.showAllCriticalPath = showAllCriticalPath,
	};
	return &OpList;
}