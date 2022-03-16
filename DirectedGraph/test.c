#include "DirectedGraph.h"

int main(int argc, char const *argv[])
{
	srand(100);
	DGraph* dg = DGraph().create();

	puts("+++++++++++++++++++++++++");
	DGraph().showGraph(dg);

	// puts("+++++++++++++++++++++++++");
	// DGraph().showTopologicalSort(dg);

	puts("+++++++++++++++++++++++++");
	DGraph().showAllTopologicalSort(dg);

	DGraph().destroy(&dg);

	// for (int i = 0; i < 1000000; i++){
	// 	dg = DGraph().create();
	// 	DGraph().destroy(&dg);
	// }
	return 0;
}