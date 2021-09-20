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

static const inline void* pop(DlQueue* queue)
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

static inline void for_each(DlQueue* queue, DucLFEFuncT func)
{
	DucList().for_each(queue->list, func);
}

const inline DlQueueOp* GetDlQueueOpStruct()
{
	static const DlQueueOp OpList = {
		.create = create,
		.clear = clear,
		.destroy = destroy,
		.pop = pop,
		.push = push,
		.empty = empty,
		.for_each = for_each,
	};
	return &OpList;
}