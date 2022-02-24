#include "UndirectedGraph.h"

int main(int argc, char const *argv[])
{
	srand(121);
	UGraph* ug = UGraph().create();

	puts("+++++++++++++++++++++++++");
	UGraph().showGraph(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().DFSTraverse(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().DFSTraverse_stack(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().BFSTraverse(ug);

	UGraph().addEdge(ug, 0, 1, 55);
	puts("+++++++++++++++++++++++++");
	UGraph().showGraph(ug);

	UGraph().destroy(&ug);
	return 0;
}