#include "UndirectedGraph.h"

int main(int argc, char const *argv[])
{
	UGraph* ug = UGraph().create();

	puts("+++++++++++++++++++++++++");
	UGraph().showGraph(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().DFSTraverse(ug);

	puts("+++++++++++++++++++++++++");
	UGraph().DFSTraverse_stack(ug);

	UGraph().destroy(&ug);
	return 0;
}