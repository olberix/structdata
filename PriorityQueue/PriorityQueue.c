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
		SqList().swap(parent_i, basis);
		if (!parent_i)
			break;
		basis = parent_i;
	}
}

static inline void __heap_adjust_down(PriorityQueue* queue, size_t basis)
{
	while(true){
		size_t parent_i = (basis - 1) / 2;
		if (!queue->topFunc(SqList().at(queue->list, parent_i), SqList().at(queue->list, basis)))
			break;
		SqList().swap(parent_i, basis);
		if (!parent_i)
			break;
		basis = parent_i;
	}
}

static inline void push(PriorityQueue* queue, const void* elem)
{
	SqList().push_back(queue->list, elem);
	__heap_adjust_up(queue, SqList().length(queue->list) - 1);
}

static inline void change(PriorityQueue* queue, size_t loc, const void* elem)
{
	// SqList().change(queue->list, loc, elem);
	// __heap_adjust_up(queue, SqList().length(queue->list) - 1);
}

static inline const void* get_top(PriorityQueue* queue)
{
	return SqList().at(queue->list, 0);
}

static inline void for_each(PriorityQueue* queue, SequenceForEachFunc_Const func, void* args)
{
	size_t length = SqList().length(queue->list);
	if (length == 0)
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
		.top = top,
		.get_top = get_top,
		.for_each = for_each,
	};
	return &OpList;
}