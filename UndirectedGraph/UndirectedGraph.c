#include "UndirectedGraph.h"
#include "../SkipList/SkipList.h"
#include "../DlQueue/DlQueue.h"
#include "../SqStack/SqStack.h"
#include "time.h"

static UGraph* create()
{
	POINTCREATE_INIT(UGraph*, g, UGraph, sizeof(UGraph));
	int edgeNum = 0;
	for (int j = 0; j < UG_MAX_VERTEX_NUM; j++){
		UGVertexNode vex;
		vex.data = j;
		vex.firstEdge = NULL;
		for (int i = 0; i < j; i++){
			if (rand() % 10000 <= UG_GEN_EDGE_RATE * (i + 1) / (j + 1)){
				POINTCREATE_INIT(UGEdgeNode*, enode, UGEdgeNode, sizeof(UGEdgeNode));
				enode->ivex = i;
				enode->jvex = j;
				enode->weight = rand() % UG_MAX_WEIGHT + 1;
				edgeNum++;
				printf("gen--[v%d v%d %d]\n", i, j, enode->weight);

				UGVertexNode i_vex = g->adjmulist[i];
				if (i_vex.firstEdge){
					UGEdgeNode* last = i_vex.firstEdge;
					while(true){
						if (last->ivex == i){
							if (last->ilink)
								last = last->ilink;
							else{
								last->ilink = enode;
								break;
							}
						}
						else{
							if (last->jlink)
								last = last->jlink;
							else{
								last->jlink = enode;
								break;
							}
						}
					}
				}
				else
					i_vex.firstEdge = enode;
				g->adjmulist[i] = i_vex;
				
				if (vex.firstEdge){
					UGEdgeNode* last = vex.firstEdge;
					while(last->jlink)
						last = last->jlink;
					last->jlink = enode;
				}
				else
					vex.firstEdge = enode;
			}
		}
		g->adjmulist[j] = vex;
	}
	g->vexNum = UG_MAX_VERTEX_NUM;
	g->edgeNum = edgeNum;

	return g;
}

//DFS
static void __destroy(UGraph* graph, UGEdgeNode* edge, SkipList* skl)
{
	if (!edge || SkipList().find(skl, &edge) != -1)
		return;
	__destroy(graph, edge->ilink, skl);
	__destroy(graph, edge->jlink, skl);

	SkipList().insert(skl, &edge);
	FREE(edge);
}

static void destroy(UGraph** sgraph)
{
	if (!(*sgraph)->edgeNum)
		return;

	SkipList* skl = SkipList().create(sizeof(UGEdgeNode*), default_equal_func_uint64, default_less_func_uint64);
	for (int i = 0; i < (*sgraph)->vexNum; i++)
		__destroy(*sgraph, (*sgraph)->adjmulist[i].firstEdge, skl);
	FREE(*sgraph);
	SkipList().destroy(&skl);
}

//BFS
static void showGraph(UGraph* graph)
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
	SkipList* skl = SkipList().create(sizeof(UGEdgeNode*), default_equal_func_uint64, default_less_func_uint64);
	DlQueue* queue = DlQueue().create(sizeof(UGEdgeNode*));
	for (int i = 0; i < graph->vexNum; i++){
		UGEdgeNode* firstEdge = graph->adjmulist[i].firstEdge;
		if (firstEdge){
			DlQueue().push(queue, &firstEdge);
			while(!DlQueue().empty(queue)){
				UGEdgeNode* edge = TOCONSTANT(UGEdgeNode*, DlQueue().pop(queue));
				if (!edge || SkipList().find(skl, &edge) != -1)
					continue;
				SkipList().insert(skl, &edge);
				printf("[v%d v%d %d]\t", edge->ivex, edge->jvex, edge->weight);
				DlQueue().push(queue, &(edge->ilink));
				DlQueue().push(queue, &(edge->jlink));
			}
		}
	}
	puts("");
	SkipList().destroy(&skl);
	DlQueue().destroy(&queue);
}

static void __DFSTraverse(UGraph* graph, UGEdgeNode* edge, bool visited[])
{
	if (!edge)
		return;
	if (!visited[edge->ivex]){
		visited[edge->ivex] = true;
		printf("v%d:%d ", edge->ivex, graph->adjmulist[edge->ivex].data);
	}
	__DFSTraverse(graph, edge->ilink, visited);
	if (!visited[edge->jvex]){
		visited[edge->jvex] = true;
		printf("v%d:%d ", edge->jvex, graph->adjmulist[edge->jvex].data);
	}
	__DFSTraverse(graph, edge->jlink, visited);
}

//类似树的前中后序遍历，对顶点不同的访问次序会导致最终的顶点序列不同
//可以发现，对于邻接多重表实现无向图，因为边的交叉指向，使用DFS/BFS对顶点进行遍历时会对同一条边进行多次重复遍历
//在这方面效率不如邻接表的实现，不过可以在一次递归内连续访问两个顶点，然后
static void DFSTraverse(UGraph* graph)
{
	if (!graph->vexNum)
		return;
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	for (int i = 0; i < graph->vexNum; i++){
		if (!visited[i]){
			visited[i] = true;
			printf("v%d:%d ", i, graph->adjmulist[i].data);
		}
		__DFSTraverse(graph, graph->adjmulist[i].firstEdge, visited);
	}
	puts("");
}

//访问次序与递归实现保持一致，因为访问
static void DFSTraverse_stack(UGraph* graph)
{
	if (!graph->vexNum)
		return;
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	SqStack* stack = SqStack().create(sizeof(UGEdgeNode*), NULL);
	for (int i = 0; i < graph->vexNum; i++){
		if (!visited[i]){
			visited[i] = true;
			printf("v%d:%d ", i, graph->adjmulist[i].data);
		}
		UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
		while (edge || !SqStack().empty(stack)){
			if (edge){
				if (!visited[edge->ivex]){
					visited[edge->ivex] = true;
					printf("v%d:%d ", edge->ivex, graph->adjmulist[edge->ivex].data);
				}
				SqStack().push(stack, &edge);
				edge = edge->ilink;
			}
			else{
				edge = TOCONSTANT(UGEdgeNode*, SqStack().pop(stack));
				if (!visited[edge->jvex]){
					visited[edge->jvex] = true;
					printf("v%d:%d ", edge->jvex, graph->adjmulist[edge->jvex].data);
				}
				edge = edge->jlink;
			}

		}
	}
	puts("");
	SqStack().destroy(&stack);
}

static void BFSTraverse(UGraph* graph)
{
	if (!graph->vexNum)
		return;

}

inline const UGraphOp* GetUGraphOpStruct()
{
	static const UGraphOp OpList = {
		.create = create,
		.destroy = destroy,
		.showGraph = showGraph,
		.DFSTraverse = DFSTraverse,
		.DFSTraverse_stack = DFSTraverse_stack,
		.BFSTraverse = BFSTraverse,
	};
	return &OpList;
}