#include "DirectedGraph.h"

int main(int argc, char const *argv[])
{
	srand(10);
	// for (int i = 0; i < 1000000; i++){
		DGraph* dg = DGraph().create();

		DGraph().addEdge(dg, 0, 1, 6);
		DGraph().addEdge(dg, 0, 2, 6);
		DGraph().addEdge(dg, 0, 3, 5);
		DGraph().addEdge(dg, 1, 4, 1);
		DGraph().addEdge(dg, 2, 4, 1);
		DGraph().addEdge(dg, 4, 6, 9);
		DGraph().addEdge(dg, 4, 7, 7);
		DGraph().addEdge(dg, 7, 8, 4);
		DGraph().addEdge(dg, 6, 8, 2);
		DGraph().addEdge(dg, 3, 5, 5);
		DGraph().addEdge(dg, 5, 7, 4);

		puts("+++++++++++++++++++++++++");
		DGraph().showGraph(dg);

		puts("+++++++++++++++++++++++++");
		DGraph().showAllTopologicalSort(dg);

		puts("+++++++++++++++++++++++++");
		DGraph().showAllCriticalPath(dg);

		DGraph().destroy(&dg);
	// }
	return 0;
}