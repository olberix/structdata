#include "UndirectedGraph.h"

int main(int argc, char const *argv[])
{
	srand(121);
	UGraph* ug = UGraph().create();

	// puts("+++++++++++++++++++++++++");
	// UGraph().showGraph(ug);

	// puts("+++++++++++++++++++++++++");
	// UGraph().DFSTraverse(ug);

	// puts("+++++++++++++++++++++++++");
	// UGraph().DFSTraverse_stack(ug);

	// puts("+++++++++++++++++++++++++");
	// UGraph().BFSTraverse(ug);
	// printf("\n\n\n");

	UGraph().addEdge(ug, 0, 5, 55);
	UGraph().deleteEdge(ug, 1, 3);
	UGraph().deleteEdge(ug, 2, 3);
	UGraph().deleteEdge(ug, 2, 4);
	UGraph().deleteEdge(ug, 3, 4);
	UGraph().deleteEdge(ug, 5, 6);
	UGraph().deleteEdge(ug, 3, 7);
	UGraph().deleteEdge(ug, 4, 8);
	UGraph().deleteEdge(ug, 5, 8);
	UGraph().deleteEdge(ug, 5, 9);
	UGraph().deleteEdge(ug, 8, 9);
	UGraph().deleteEdge(ug, 5, 10);
	UGraph().deleteEdge(ug, 10, 11);
	UGraph().deleteEdge(ug, 6, 12);
	UGraph().deleteEdge(ug, 10, 12);
	UGraph().deleteEdge(ug, 11, 12);
	UGraph().deleteEdge(ug, 9, 13);
	UGraph().deleteEdge(ug, 4, 14);
	UGraph().deleteEdge(ug, 6, 14);
	UGraph().deleteEdge(ug, 0, 5);

	UGraph().addEdge(ug, 0, 1, 55);
	UGraph().addEdge(ug, 1, 2, 55);
	UGraph().addEdge(ug, 2, 3, 55);
	UGraph().addEdge(ug, 3, 4, 55);
	UGraph().addEdge(ug, 4, 5, 55);
	UGraph().addEdge(ug, 5, 6, 55);
	UGraph().addEdge(ug, 6, 7, 55);
	UGraph().addEdge(ug, 7, 8, 55);
	UGraph().addEdge(ug, 8, 9, 55);
	UGraph().addEdge(ug, 9, 10, 55);
	UGraph().addEdge(ug, 10, 11, 55);
	UGraph().addEdge(ug, 11, 12, 55);
	UGraph().addEdge(ug, 12, 13, 55);
	UGraph().addEdge(ug, 13, 14, 55);
	UGraph().addEdge(ug, 14, 0, 55);

	puts("+++++++++++++++++++++++++");
	UGraph().showGraph(ug);
	puts("+++++++++++++++++++++++++");
	UGraph().DFSTraverse(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().DFSTraverse_stack(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().BFSTraverse(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().isConnected(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().showDegree(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().hasCycle(ug);

	UGraph().destroy(&ug);
	return 0;
}