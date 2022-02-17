#include "PriorityQueue.h"

static PriorityQueue* create(size_t ESize, CmnCompareFunc topFunc)
{
	POINTCREATE(PriorityQueue*, queue, PriorityQueue, sizeof(PriorityQueue));
	queue->list = SqList().create(ESize, NULL);
	queue->topFunc = topFunc;
	return queue;
}

static inline void destroy(PriorityQueue** sQueue)
{
	SqList().destroy(&((*sQueue)->list));
	FREE((*sQueue));
}

static inline void clear(PriorityQueue* queue)
{
	SqList().clear(queue->list);
}

static inline size_t length(PriorityQueue* queue)
{
	return SqList().length(queue->list);
}

static inline bool empty(PriorityQueue* queue)
{
	return SqList().length(queue->list) == 0;
}

static inline void __heap_adjust_up(PriorityQueue* queue, size_t basis)
{
	if (!basis)
		return;
	while(true){
		size_t parent_i = (basis - 1) / 2;
		if (!queue->topFunc(SqList().at(queue->list, parent_i), SqList().at(queue->list, basis)))
			break;
		SqList().swap(queue->list, parent_i, basis);
		if (!parent_i)
			break;
		basis = parent_i;
	}
}

static inline void __heap_adjust_down(PriorityQueue* queue, size_t basis)
{
	while(true){
		size_t left_i = (basis << 1) + 1;
		const void* lhs = SqList().at(queue->list, left_i);
		if (!lhs)
			break;
		const void* topper = lhs;
		size_t next = left_i;
		size_t right_i = (basis << 1) + 2;
		const void* rhs = SqList().at(queue->list, right_i);
		if (rhs && queue->topFunc(lhs, rhs)){
			topper = rhs;
			next = right_i;
		}
		if (!queue->topFunc(SqList().at(queue->list, basis), topper))
			break;
		SqList().swap(queue->list, basis, next);
		basis = next;
	}
}

static inline void push(PriorityQueue* queue, const void* elem)
{
	SqList().push_back(queue->list, elem);
	__heap_adjust_up(queue, SqList().length(queue->list) - 1);
}

static inline void change(PriorityQueue* queue, size_t loc, const void* elem)
{
	if (loc >= SqList().length(queue->list))
		return;
	bool topAdj = queue->topFunc(SqList().at(queue->list, loc), elem);
	SqList().change(queue->list, loc, elem);
	if (topAdj)
		__heap_adjust_up(queue, loc);
	else
		__heap_adjust_down(queue, loc);
}

static inline const void* erase(PriorityQueue* queue, size_t loc)
{
	if (loc >= SqList().length(queue->list))
		return NULL;
	SqList().swap(queue->list, loc, SqList().length(queue->list) - 1);
	const void* ret = SqList().erase(queue->list, SqList().length(queue->list) - 1);
	__heap_adjust_down(queue, loc);
	return ret;
}

static inline const void* pop(PriorityQueue* queue)
{
	return erase(queue, 0);
}

static inline const void* get_top(PriorityQueue* queue)
{
	return SqList().at(queue->list, 0);
}

static inline void for_each(PriorityQueue* queue, SequenceForEachFunc_Const func, void* args)
{
	size_t length = SqList().length(queue->list);
	if (!length)
		return;
	SqList* list = queue->list;
	size_t e_S = list->e_S;
	for (size_t i = 0; i < length; i++)
		func(i, list->pElems + i * e_S, args);
}

inline const PriorityQueueOp* GetPriorityQueueOpStruct()
{
	static const PriorityQueueOp OpList = {
		.create = create,
		.destroy = destroy,
		.clear = clear,
		.length = length,
		.empty = empty,
		.push = push,
		.change = change,
		.erase = erase,
		.pop = pop,
		.get_top = get_top,
		.for_each = for_each,
	};
	return &OpList;
}