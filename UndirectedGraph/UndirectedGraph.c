#include "UndirectedGraph.h"
#include "time.h"

static UndirectedGraph* create()
{
	POINTCREATE_INIT(UndirectedGraph*, g, UndirectedGraph, sizeof(UndirectedGraph));
	for (int j = 0; j < MAX_VERTEX_NUM; j++){
		UGVertexNode vex;
		vex.data = j;
		vex.firstEdge = NULL;
		int edgeNum = 0;
		for (int i = 0; i < j; i++){
			if (rand() % 10000 <= GEN_EDGE_RATE - 1){
				POINTCREATE_INIT(UGEdgeNode*, enode, UGEdgeNode, sizeof(UGEdgeNode));
				enode->ivex = i;
				enode->jvex = j;
				enode->weight = rand() % UG_MAX_WEIGHT + 1;
				edgeNum++;
				
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
	g->vexNum = MAX_VERTEX_NUM;
	g->edgeNum = edgeNum;

	return g;
}

static void __destroy(UndirectedGraph* graph, UGEdgeNode* edge, bool visited[])
{
	if (!edge)
		return;
	__destroy(graph, edge->ilink, visited);
	__destroy(graph, edge->jlink, visited);
	
	FREE(edge);
}

static void destroy(UndirectedGraph** sgraph)
{
	if (!(*sgraph)->edgeNum)
		return;
	bool visited[UG_MAX_VERTEX_NUM];
	memset(visited, 0, sizeof(visited));
	for (int i = 0; i < (*sgraph)->vexNum; i++){
		UGEdgeNode* firstEdge = (*sgraph)->adjmulist[i].firstEdge;
		if (firstEdge)
			__destroy(*sgraph, edge);
	}
	FREE(*sgraph);
}