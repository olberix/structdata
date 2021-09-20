#include "DlQueue.h"

typedef struct StD
{
	int a;
	char b;
}StD;

int main(int argc, char const *argv[])
{
	DlQueue* queue = DlQueue().create(sizeof(StD));
	StD tmp = {10, 'd'};
	DlQueue().push(queue, &tmp);
	tmp.a = 11;
	tmp.b = 'k';
	DlQueue().push(queue, &tmp);
	tmp = TOCONSTANT(StD, DlQueue().pop(queue));
	printf("%d\t%c\n", tmp.a, tmp.b);
	tmp = TOCONSTANT(StD, DlQueue().pop(queue));
	printf("%d\t%c\n", tmp.a, tmp.b);

	return 1;
}