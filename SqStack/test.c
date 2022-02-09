#include "SqStack.h"

typedef struct StB{
	int a;
	double b;
	double c;
}StB;

int main()
{
	StB tmp = {0, 220.3, 20.1};
	size_t size = 500;
	SqStack* stack = SqStack().create(sizeof(StB), &size);
	SqStack().push(stack, &tmp);
	tmp.a = 10;
	tmp.b = 10.1;
	SqStack().push(stack, &tmp);
	tmp.a = 104;
	tmp.b = 102.1;
	SqStack().push(stack, &tmp);

	SqStack().pop(stack);
	SqStack().pop(stack);
	SqStack().pop(stack);
	SqStack().push(stack, &tmp);
	SqStack().clear(stack);

	if (SqStack().empty(stack))
		puts("empty");
	else
		puts("not empty");

	tmp.a = 1041;
	tmp.b = 1042.1;
	SqStack().push(stack, &tmp);
	SqStack().push(stack, &tmp);
	// tmp = TOCONSTANT(StB, SqStack().pop(stack));
	tmp = TOCONSTANT(StB, SqStack().get_top(stack));
	printf("%d\n", tmp.a);
	if (SqStack().empty(stack))
		puts("empty");
	else
		puts("not empty");

	SQSTACK_FOREACH(stack, StB, {
		printf("%lu\t%d\t%f\t%f\n", key, value.a, value.b, value.c);
	});
	SQSTACK_FOREACH_REVERSE(stack, StB, {
		printf("%lu\t%d\t%f\t%f\n", key, value.a, value.b, value.c);
	});

	SqStack().destroy(&stack);
	printf("%p\n", stack);

	return 0;
}