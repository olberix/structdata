#include "PriorityQueue.h"

bool top_func(const void* lhs, const void* rhs)
{
	return !default_less_func_int32(lhs, rhs);
}

void display(size_t index, const void* elem, void* args)
{
	const int* _elem = elem;
	printf("%d--+++++++++++++++\n", *_elem);
}

int main(int argc, char const *argv[])
{
	PriorityQueue* queue = PriorityQueue().create(sizeof(int), top_func);

	int range = 15;
	for (int i = 0; i < range; i += 3){
		PriorityQueue().push(queue, &i);
	}
	for (int i = 1; i <= range; i += 3){
		PriorityQueue().push(queue, &i);
	}
	for (int i = 2; i <= range; i += 3){
		PriorityQueue().push(queue, &i);
	}


	PriorityQueue().for_each(queue, display, NULL);
	puts("+++++++++++++++++++++++++++++++++");
	// int i = 10;
	// PriorityQueue().change(queue, 0, &i);
	// i = 12;
	// PriorityQueue().change(queue, 1, &i);

	PriorityQueue().erase(queue, 0);
	PriorityQueue().erase(queue, 2);
	PriorityQueue().erase(queue, 3);

	int length = PriorityQueue().length(queue);
	for (int i = 0; i < length; i++){
		const void* ret = PriorityQueue().pop(queue);
		int elem = TOCONSTANT(int, ret);
		printf("%d------\n", elem);
		// PriorityQueue().for_each(queue, display, NULL);
	}

	PriorityQueue().destroy(&queue);
	return 0;
}