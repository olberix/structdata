#include "UndirectedGraph.h"
#include "../SkipList/SkipList.h"
#include "../DlQueue/DlQueue.h"
#include "../SqStack/SqStack.h"
#include "../PriorityQueue/PriorityQueue.h"
#include <time.h>
#include <limits.h>

static UGraph* create()
{
	POINTCREATE_INIT(UGraph*, g, UGraph, sizeof(UGraph));
	int edgeNum = 0;
	for (int j = 0; j < UG_MAX_VERTEX_NUM; j++){
		UGVertexNode vex;
		vex.data = j;
		vex.firstEdge = NULL;
		for (int i = 0; i < j; i++){
			if (rand() % 10000 > UG_GEN_EDGE_RATE * (i + 1) / (j + 1))
				continue;
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
	int count = 0;
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
				count++;
			}
		}
	}
	puts("");
	printf("total edge output:%d\n", count);
	SkipList().destroy(&skl);
	DlQueue().destroy(&queue);
}

static void __DFSTraverse(UGraph* graph, int i, UGEdgeNode* edge, SkipList* skl, bool visited[], int* count)
{
	if (!visited[i]){
		visited[i] = true;
		(*count)++;
		printf("v%d:%d ", i, graph->adjmulist[i].data);
	}
	if (!edge){
		if (graph->adjmulist[i].firstEdge)
			__DFSTraverse(graph, i, graph->adjmulist[i].firstEdge, skl, visited, count);
		return;
	}
	if (SkipList().find(skl, &edge) != -1)
		return;
	SkipList().insert(skl, &edge);
	if (edge->ivex == i){
		__DFSTraverse(graph, edge->jvex, edge->jlink, skl, visited, count);
		__DFSTraverse(graph, edge->ivex, edge->ilink, skl, visited, count);
	}
	else{
		__DFSTraverse(graph, edge->ivex, edge->ilink, skl, visited, count);
		__DFSTraverse(graph, edge->jvex, edge->jlink, skl, visited, count);
	}
}

//类似树的前中后序遍历，对顶点不同的访问次序会导致最终的顶点序列不同
//可以发现，对于邻接多重表实现无向图，可以使用DFS/BFS分别对边或者顶点进行遍历
//上面destroy和showGraph是对边的DFS/BFS遍历，下面是对顶点的的DFS/BFS遍历，对边和顶点的遍历可以产生不同的顶点序列
static void DFSTraverse(UGraph* graph)
{
	if (!graph->vexNum)
		return;
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	SkipList* skl = SkipList().create(sizeof(UGEdgeNode*), default_equal_func_uint64, default_less_func_uint64);
	int count = 0;
	for (int i = 0; i < graph->vexNum; i++)
		__DFSTraverse(graph, i, graph->adjmulist[i].firstEdge, skl, visited, &count);
	printf("\nDFS_recur total vertex output:%d\n", count);
	SkipList().destroy(&skl);
}

//访问次序与递归实现保持一致
//邻接多重表DFS的栈实现让人头痛,要分清对边和顶点的遍历
static void DFSTraverse_stack(UGraph* graph)
{
	if (!graph->vexNum)
		return;

	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	SqStack* stack_edge = SqStack().create(sizeof(UGEdgeNode*), NULL);
	SqStack* stack_vex = SqStack().create(sizeof(int), NULL);
	SkipList* skl = SkipList().create(sizeof(UGEdgeNode*), default_equal_func_uint64, default_less_func_uint64);

	int count = 0;
	for (int i = 0; i < graph->vexNum; i++){
		UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
		int curVex = i;

		while (true){
			if (!visited[curVex]){
				visited[curVex] = true;
				printf("v%d:%d ", curVex, graph->adjmulist[curVex].data);
				count++;
			}

			if (!edge && graph->adjmulist[curVex].firstEdge){
				edge = graph->adjmulist[curVex].firstEdge;
			}
			else if (!edge || (edge && SkipList().find(skl, &edge) != -1)){
				if (SqStack().empty(stack_edge))
					break;
				edge = TOCONSTANT(UGEdgeNode*, SqStack().pop(stack_edge));
				if (TOCONSTANT(int, SqStack().pop(stack_vex)) == edge->ivex){
					curVex = edge->jvex;
					edge = edge->jlink;
				}
				else{
					curVex = edge->ivex;
					edge = edge->ilink;
				}
			}
			else if (edge){
				SkipList().insert(skl, &edge);
				SqStack().push(stack_edge, &edge);
				if (edge->ivex == curVex){
					curVex = edge->jvex;
					SqStack().push(stack_vex, &curVex);
					edge = edge->jlink;
				}
				else{
					curVex = edge->ivex;
					SqStack().push(stack_vex, &curVex);
					edge = edge->ilink;
				}
			}
			if (!edge && SqStack().empty(stack_edge))
				break;
		}
	}
	printf("\nDFS_statck total vertex output:%d\n", count);
	SqStack().destroy(&stack_edge);
	SqStack().destroy(&stack_vex);
	SkipList().destroy(&skl);
}

static void BFSTraverse(UGraph* graph)
{
	if (!graph->vexNum)
		return;
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	DlQueue* queue = DlQueue().create(sizeof(int));
	int count = 0;
	for (int i = 0; i < graph->vexNum; i++){
		if (!visited[i]){
			DlQueue().push(queue, &i);
			while(!DlQueue().empty(queue)){
				int curVex = TOCONSTANT(int, DlQueue().pop(queue));
				if (!visited[curVex]){
					visited[curVex] = true;
					printf("v%d:%d ", curVex, graph->adjmulist[curVex].data);
					count++;
				}
				UGEdgeNode* edge = graph->adjmulist[curVex].firstEdge;
				while(edge){
					if (edge->ivex == curVex){
						if (!visited[edge->jvex])
							DlQueue().push(queue, &(edge->jvex));
						edge = edge->ilink;
					}
					else{
						if (!visited[edge->ivex])
							DlQueue().push(queue, &(edge->ivex));
						edge = edge->jlink;
					}
				}
			}
		}
	}
	printf("\nBFS total vertex output:%d\n", count);
	DlQueue().destroy(&queue);
}

static inline void __set_connection(UGraph* graph, UGEdgeNode* priorEdge, UGEdgeNode* addEdge, int vex)
{
	if (priorEdge){
		if (priorEdge->ivex == vex)
			priorEdge->ilink = addEdge;
		else
			priorEdge->jlink = addEdge;
	}
	else
		graph->adjmulist[vex].firstEdge = addEdge;
}

static void addEdge(UGraph* graph, int vex_1, int vex_2, int weight)
{
	if (vex_1 >= graph->vexNum || vex_2 >= graph->vexNum || vex_1 == vex_2)
		return;
	UGEdgeNode* edge = graph->adjmulist[vex_1].firstEdge;
	while(edge){//判断重复边
		if (edge->ivex == vex_1){
			if (edge->jvex == vex_2)
				return;
			if (!edge->ilink)
				break;
			edge = edge->ilink;
		}
		else{
			if (edge->ivex == vex_2)
				return;
			if (!edge->jlink)
				break;
			edge = edge->jlink;
		}
	}

	POINTCREATE_INIT(UGEdgeNode*, enode, UGEdgeNode, sizeof(UGEdgeNode));
	enode->ivex = vex_1;
	enode->jvex = vex_2;
	enode->weight = weight;
	__set_connection(graph, edge, enode, vex_1);

	edge = graph->adjmulist[vex_2].firstEdge;
	while(edge){
		if (edge->ivex == vex_2){
			if (!edge->ilink)
				break;
			edge = edge->ilink;
		}
		else{
			if (!edge->jlink)
				break;
			edge = edge->jlink;
		}
	}
	__set_connection(graph, edge, enode, vex_2);

	graph->edgeNum++;
}

static inline void __reset_connection(UGraph* graph, UGEdgeNode* priorEdge, UGEdgeNode* delEdge, int vex)
{
	if (priorEdge){
		if (priorEdge->ivex == vex){
			if (delEdge->ivex == vex)
				priorEdge->ilink = delEdge->ilink;
			else
				priorEdge->ilink = delEdge->jlink;
		}
		else{
			if (delEdge->ivex == vex)
				priorEdge->jlink = delEdge->ilink;
			else
				priorEdge->jlink = delEdge->jlink;
		}
	}
	else{
		if (delEdge->ivex == vex)
			graph->adjmulist[vex].firstEdge = delEdge->ilink;
		else
			graph->adjmulist[vex].firstEdge = delEdge->jlink;
	}
}

static void deleteEdge(UGraph* graph, int vex_1, int vex_2)
{
	if (vex_1 >= graph->vexNum || vex_2 >= graph->vexNum || vex_1 == vex_2)
		return;
	UGEdgeNode* priorEdge_1 = NULL;
	UGEdgeNode* edge = graph->adjmulist[vex_1].firstEdge;
	while(edge){
		if (edge->ivex == vex_1){
			if (edge->jvex == vex_2)
				break;
			priorEdge_1 = edge;
			edge = edge->ilink;
		}
		else{
			if (edge->ivex == vex_2)
				break;
			priorEdge_1 = edge;
			edge = edge->jlink;
		}
	}
	if (!edge)
		return;
	__reset_connection(graph, priorEdge_1, edge, vex_1);
	
	UGEdgeNode* priorEdge_2 = NULL;
	UGEdgeNode* tmpEdge = graph->adjmulist[vex_2].firstEdge;
	while(tmpEdge){
		if (tmpEdge == edge)
			break;
		priorEdge_2 = tmpEdge;
		if (tmpEdge->ivex == vex_2)
			tmpEdge = tmpEdge->ilink;
		else
			tmpEdge = tmpEdge->jlink;
	}
	__reset_connection(graph, priorEdge_2, edge, vex_2);

	FREE(edge);
	graph->edgeNum--;
}

static int UNION_ROOT(int union_set[], int vex)
{
	return vex == union_set[vex] ? vex : (union_set[vex] = UNION_ROOT(union_set, union_set[vex]));
}

static inline void UNION_MERGE(int union_set[], int vex_1, int vex_2)
{
	union_set[UNION_ROOT(union_set, vex_1)] = union_set[UNION_ROOT(union_set, vex_2)];
}

//判断无向图的连通性方法DFS BFS union-find warshell(比较适合邻接矩阵实现)
//这里采用union-find方式,对于邻接多重表实现的无向图来说,这里事实上也对边进行了DFS或BFS遍历
static bool isConnected(UGraph* graph)
{
	if (!graph->vexNum)
		return false;
	int union_set[UG_MAX_VERTEX_NUM];
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++)
		union_set[i] = i;
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
				UNION_MERGE(union_set, edge->ivex, edge->jvex);
				DlQueue().push(queue, &(edge->ilink));
				DlQueue().push(queue, &(edge->jlink));
			}
		}
	}
	//其实这里可以输出所有连通分量,懒得折腾了
	int rootVex = UNION_ROOT(union_set, 0);
	bool ret = true;
	for (int i = 1; i < UG_MAX_VERTEX_NUM; i++){
		if (UNION_ROOT(union_set, i) != rootVex){
			ret = false;
			break;
		}
	}
	SkipList().destroy(&skl);
	DlQueue().destroy(&queue);
	return ret;
}

static inline int* __get_degree(UGraph* graph)
{
	static int degree_array[UG_MAX_VERTEX_NUM];
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++){
		UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
		int degree = 0;
		while(edge){
			degree++;
			if (i == edge->ivex)
				edge = edge->ilink;
			else
				edge = edge->jlink;
		}
		degree_array[i] = degree;
	}
	return degree_array;
}

//无向图判断有环,这里采用第二种方法
//1.DFS(对边进行DFS遍历,遇到已经访问过的边必然有环)
//2.拓扑排序思想
//3.并查集,在这里并查集的本质是基于DFS的实现,遍历边是查看两个顶点父结点是否相同,本质就是对边进行DFS访问时是否遇到访问过的边
static bool hasCycle(UGraph* graph)
{
	if (graph->edgeNum >= graph->vexNum)
		return true;
	int* degree = __get_degree(graph);
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++){
		if (degree[i] == 1){
			degree[i] = 0;
			UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
			while(edge){
				if (i == edge->ivex){
					degree[edge->jvex] -= 1;
					edge = edge->ilink;
				}
				else{
					degree[edge->ivex] -= 1;
					edge = edge->jlink;
				}
			}
			i = 0;
		}
	}
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++)
		if (degree[i] > 0)
			return true;

	return false;
}

static void showDegree(UGraph* graph)
{
	int* degree = __get_degree(graph);
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++)
		printf("TD(v%d):%d ", i, degree[i]);
	puts("");
}

static inline bool __top_func(const void* lhs, const void* rhs)
{
	return (*(UGEdgeNode**)rhs)->weight < (*(UGEdgeNode**)lhs)->weight;
}

//并查集,跳表,优先队列不判重
static void showMiniSpanTree_Kruskal(UGraph* graph)
{
	if (!isConnected(graph)){
		puts("no minispantree in unconnected graph.");
		return;
	}
	SkipList* skl = SkipList().create(sizeof(UGEdgeNode*), default_equal_func_uint64, default_less_func_uint64);
	PriorityQueue* pQ = PriorityQueue().create(sizeof(UGEdgeNode*), __top_func);
	for (int i = 0; i < graph->vexNum; i++){
		UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
		while(edge){
			if (SkipList().find(skl, &edge) == -1){
				SkipList().insert(skl, &edge);
				PriorityQueue().push(pQ, &edge);
			}
			if (edge->ivex == i)
				edge = edge->ilink;
			else
				edge = edge->jlink;
		}
	}

	int union_set[UG_MAX_VERTEX_NUM];
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++)
		union_set[i] = i;
	UGEdgeNode* edge_set[UG_MAX_VERTEX_NUM];
	int edge_idx = 0;
	while (edge_idx < UG_MAX_VERTEX_NUM - 1){
		UGEdgeNode* edge = TOCONSTANT(UGEdgeNode*, PriorityQueue().pop(pQ));
		if (UNION_ROOT(union_set, edge->ivex) == UNION_ROOT(union_set, edge->jvex))
			continue;
		edge_set[edge_idx++] = edge;
		UNION_MERGE(union_set, edge->ivex, edge->jvex);
	}
	puts("showMiniSpanTree_Kruskal:");
	int tW = 0;
	for (int i = 0; i < graph->vexNum - 1; i++){
		printf("[v%d v%d %d]\t", edge_set[i]->ivex, edge_set[i]->jvex, edge_set[i]->weight);
		tW += edge_set[i]->weight;
	}
	printf("\ntotal weight:%d\n", tW);
	PriorityQueue().destroy(&pQ);
	SkipList().destroy(&skl);
}

static void showMiniSpanTree_Prim(UGraph* graph)
{
	if (!isConnected(graph)){
		puts("no minispantree in unconnected graph.");
		return;
	}
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	visited[0] = true;
	UGEdgeNode* edge_set[UG_MAX_VERTEX_NUM];
	int edge_idx = 0;
	while (edge_idx != UG_MAX_VERTEX_NUM - 1){
		UGEdgeNode* edge_rec = NULL;
		int weight = INT_MAX;
		for (int i = 0; i < UG_MAX_VERTEX_NUM; i++){
			if (!visited[i])
				continue;
			UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
			while (edge){
				if (edge->ivex == i){
					if (!visited[edge->jvex] && edge->weight < weight){
						edge_rec = edge;
						weight = edge->weight;
					}
					edge = edge->ilink;
				}
				else{
					if (!visited[edge->ivex] && edge->weight < weight){
						edge_rec = edge;
						weight = edge->weight;
					}
					edge = edge->jlink;
				}
			}
		}
		if (edge_rec){
			edge_set[edge_idx++] = edge_rec;
			visited[edge_rec->ivex] = true;
			visited[edge_rec->jvex] = true;
		}
	}
	puts("showMiniSpanTree_Prim:");
	int tW = 0;
	for (int i = 0; i < graph->vexNum - 1; i++){
		printf("[v%d v%d %d]\t", edge_set[i]->ivex, edge_set[i]->jvex, edge_set[i]->weight);
		tW += edge_set[i]->weight;
	}
	printf("\ntotal weight:%d\n", tW);
}

typedef struct UG_VEXWEIGHT{
	int _vex;
	int _weight;
}UG_VEXWEIGHT;

static inline bool __ugvw_top_func(const void* lhs, const void* rhs)
{
	return ((UG_VEXWEIGHT*)rhs)->_weight < ((UG_VEXWEIGHT*)lhs)->_weight;
}

void showShortestPath_Dijkstra(UGraph* graph, int vex)
{
	if (vex < 0 || vex >= graph->vexNum){
		puts("invalid vex.");
		return;
	}
	if (!graph->edgeNum){
		puts("empty edge set.");
		return;
	}
	int path_rec[UG_MAX_VERTEX_NUM];
	int path_weight[UG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++)
		path_weight[i] = INT_MAX;
	path_weight[vex] = 0;
	path_rec[vex] = vex;
	UG_VEXWEIGHT _vw = {._vex = vex, ._weight = path_weight[vex]};
	PriorityQueue* pQ = PriorityQueue().create(sizeof(UG_VEXWEIGHT), __ugvw_top_func);
	PriorityQueue().push(pQ, &_vw);
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	
	while(!PriorityQueue().empty(pQ)){
		_vw = TOCONSTANT(UG_VEXWEIGHT, PriorityQueue().pop(pQ));
		if (visited[_vw._vex])
			continue;
		visited[_vw._vex] = true;
		UGEdgeNode* edge = graph->adjmulist[_vw._vex].firstEdge;
		while(edge){
			int tW = _vw._weight + edge->weight;
			if (edge->ivex == _vw._vex){
				if (path_weight[edge->jvex] > tW){
					path_rec[edge->jvex] = edge->ivex;
					path_weight[edge->jvex] = tW;
					UG_VEXWEIGHT tmp = {._vex = edge->jvex, ._weight = tW};
					PriorityQueue().push(pQ, &tmp);
				}
				edge = edge->ilink;
			}
			else{
				if (path_weight[edge->ivex] > tW){
					path_rec[edge->ivex] = edge->jvex;
					path_weight[edge->ivex] = tW;
					UG_VEXWEIGHT tmp = {._vex = edge->ivex, ._weight = tW};
					PriorityQueue().push(pQ, &tmp);
				}
				edge = edge->jlink;
			}
		}
	}

	printf("shortestPath_Dijkstra from v%d to the others:\n", vex);
	for (int i = 0; i < graph->vexNum; i++){
		if (path_weight[i] == INT_MAX){
			printf("v%d-->v%d total weight:INF path:(nil)\n", vex, i);
			continue;
		}
		int path[UG_MAX_VERTEX_NUM];
		int idx = 0, loc = i;
		path[idx++] = loc;
		while(loc != vex){
			loc = path_rec[loc];
			path[idx++] = loc;
		}
		printf("v%d-->v%d total weight:%d path:(", vex, i, path_weight[i]);
		for (int j = idx - 1; j >= 0; j--){
			if (j == 0)
				printf("%d)\n", path[j]);
			else
				printf("%d, ", path[j]);
		}
	}

	PriorityQueue().destroy(&pQ);
}

static void showShortestPath_Floyd(UGraph* graph)
{
	if (!graph->edgeNum){
		puts("empty edge set.");
		return;
	}
	int path_rec[UG_MAX_VERTEX_NUM][UG_MAX_VERTEX_NUM];
	int path_weight[UG_MAX_VERTEX_NUM][UG_MAX_VERTEX_NUM];
	for (int i = 0; i < graph->vexNum; i++)
		for (int j = 0; j < graph->vexNum; j++){
			if (i == j){
				path_weight[i][i] = 0;
				path_rec[i][i] = i;
			}
			else{
				path_weight[i][j] = INT_MAX;
				path_rec[i][j] = -1;
			}
		}
	for (int i = 0; i < graph->vexNum; i++){
		UGEdgeNode* edge = graph->adjmulist[i].firstEdge;
		while(edge){
			path_weight[edge->ivex][edge->jvex] = edge->weight;
			path_weight[edge->jvex][edge->ivex] = edge->weight;
			path_rec[edge->ivex][edge->jvex] = edge->ivex;
			path_rec[edge->jvex][edge->ivex] = edge->jvex;
			if (edge->ivex == i)
				edge = edge->ilink;
			else
				edge = edge->jlink;
		}
	}
	for (int k = 0; k < graph->vexNum; k++)
		for (int i = 0; i < graph->vexNum; i++)
			for (int j = 0; j < graph->vexNum; j++)
				if ((long long)path_weight[i][j] > (long long)path_weight[i][k] + (long long)path_weight[k][j]){
					path_weight[i][j] = path_weight[i][k] + path_weight[k][j];
					path_rec[i][j] = path_rec[k][j];
				}
	printf("shortestPath_Floyd between two points:\n");
	for (int i = 0; i < graph->vexNum; i++)
		for (int j = i + 1; j < graph->vexNum; j++){
			if (path_weight[i][j] == INT_MAX){
				printf("v%d-->v%d total weight:INF path:(nil)\n", i, j);
				continue;
			}
			printf("v%d-->v%d total weight:%d path:(%d, ", i, j, path_weight[i][j], i);
			int path = path_rec[j][i];
			while(true){
				if (path == j){
					printf("%d)\n", j);
					break;
				}
				printf("%d, ", path);
				path = path_rec[j][path];
			}
		}
}

static void showShortestPath_BFS(UGraph* graph, int begin_vex, int end_vex)
{
	if (!graph->edgeNum){
		puts("empty edge set.");
		return;
	}
	if (begin_vex < 0 || begin_vex >= graph->vexNum || end_vex < 0 || end_vex >= graph->vexNum || begin_vex == end_vex){
		puts("invalid vex.");
		return;
	}
	int path_rec[UG_MAX_VERTEX_NUM];
	path_rec[begin_vex] = -1;
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	DlQueue* queue = DlQueue().create(sizeof(int));
	DlQueue().push(queue, &end_vex);
	while(!DlQueue().empty(queue)){
		int vex = TOCONSTANT(int, DlQueue().pop(queue));
		UGEdgeNode* edge = graph->adjmulist[vex].firstEdge;
		while(edge){
			if (edge->ivex == vex){
				if (!visited[edge->jvex]){
					path_rec[edge->jvex] = edge->ivex;
					if (edge->jvex == begin_vex)
						break;
					visited[edge->jvex] = true;
					DlQueue().push(queue, &(edge->jvex));
				}
				edge = edge->ilink;
			}
			else{
				if (!visited[edge->ivex]){
					path_rec[edge->ivex] = edge->jvex;
					if (edge->ivex == begin_vex)
						break;
					visited[edge->ivex] = true;
					DlQueue().push(queue, &(edge->ivex));
				}
				edge = edge->jlink;
			}
		}
		if (path_rec[begin_vex] != -1)
			break;
	}

	printf("shortestPath_BFS from v%d to v%d:\n", begin_vex, end_vex, begin_vex);
	if (path_rec[begin_vex] != -1){
		printf("(%d, ", begin_vex);
		int path = path_rec[begin_vex];
		while(path != end_vex){
			printf("%d, ", path);
			path = path_rec[path];
		}
		printf("%d)\n", path);
	}
	else
		printf("(nil)\n");

	DlQueue().destroy(&queue);
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
		.addEdge = addEdge,
		.deleteEdge = deleteEdge,
		.isConnected = isConnected,
		.hasCycle = hasCycle,
		.showDegree = showDegree,
		.showMiniSpanTree_Kruskal = showMiniSpanTree_Kruskal,
		.showMiniSpanTree_Prim = showMiniSpanTree_Prim,
		.showShortestPath_Dijkstra = showShortestPath_Dijkstra,
		.showShortestPath_Floyd = showShortestPath_Floyd,
		.showShortestPath_BFS = showShortestPath_BFS,
	};
	return &OpList;
}