#include "DlQueue.h"

static DlQueue* create(size_t ESize)
{
	POINTCREATE(DlQueue*, ret, DlQueue, sizeof(DlQueue));
	ret->list = DucList().create(ESize);
	return ret;
}

static inline void clear(DlQueue* queue)
{
	DucList().clear(queue->list);
}

static inline void destroy(DlQueue** sQueue)
{
	DucList().destroy(&((*sQueue)->list));
	FREE((*sQueue));
}

static inline const void* pop(DlQueue* queue)
{
	return DucList().erase(queue->list, 0);
}

static inline void push(DlQueue* queue, const void* elem)
{
	return DucList().push_back(queue->list, elem);
}

static inline bool empty(DlQueue* queue)
{
	return DucList().length(queue->list) == 0;
}

static inline void for_each(DlQueue* queue, SequenceForEachFunc_Mutable func, void* args)
{
	DucList().for_each(queue->list, func, args);
}

static inline const void* at(DlQueue* queue, size_t loc)
{
	return DucList().at(queue->list, loc);
}

inline const DlQueueOp* GetDlQueueOpStruct()
{
	static const DlQueueOp OpList = {
		.create = create,
		.clear = clear,
		.destroy = destroy,
		.pop = pop,
		.push = push,
		.empty = empty,
		.for_each = for_each,
		.at = at,
	};
	return &OpList;
}