#include "DirectedGraph.h"

int main(int argc, char const *argv[])
{
	DGraph* dg = DGraph().create();

	DGraph().destroy(&dg);
	return 0;
}