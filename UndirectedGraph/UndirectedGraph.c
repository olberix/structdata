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

//判断无向图的连通性方法DFS BFS union-find warshell(比较适合邻接矩阵实现)
//这里采用union-find方式,对于邻接多重表实现的无向图来说,这里事实上也对边进行了DFS或BFS遍历
static void isConnected(UGraph* graph)
{
	if (!graph->vexNum){
		puts("empty graph.");
		return;
	}
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
				union_set[UNION_ROOT(union_set, edge->ivex)] = union_set[UNION_ROOT(union_set, edge->jvex)];
				DlQueue().push(queue, &(edge->ilink));
				DlQueue().push(queue, &(edge->jlink));
			}
		}
	}
	//其实这里可以输出所有连通分量,懒得折腾了
	int rootVex = UNION_ROOT(union_set, 0);
	for (int i = 1; i < UG_MAX_VERTEX_NUM; i++){
		if (UNION_ROOT(union_set, i) != rootVex){
			puts("this graph is not connected.");
			return;
		}
	}
	puts("this graph is connected.");
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
static void hasCycle(UGraph* graph)
{
	if (graph->edgeNum >= graph->vexNum){
		puts("this is graph is cycled.");
		return;
	}
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
		if (degree[i] > 0){
			puts("this is graph is cycled.");
			return;
		}

	puts("this is graph is not cycled.");
}

static void showDegree(UGraph* graph)
{
	int* degree = __get_degree(graph);
	for (int i = 0; i < UG_MAX_VERTEX_NUM; i++)
		printf("TD(v%d):%d ", i, degree[i]);
	puts("");
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
	};
	return &OpList;
}