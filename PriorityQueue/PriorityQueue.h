#ifndef _PRIORITY_QUEUE__
#define _PRIORITY_QUEUE__
#include "../common/common.h"
#include "../SqList/SqList.h"

typedef struct PriorityQueue{
	SqList* list;
	CmnCompareFunc topFunc;
}PriorityQueue;

typedef struct PriorityQueueOp{
	PriorityQueue* (*create)(size_t, CmnCompareFunc);
	void (*destroy)(PriorityQueue**);
	void (*clear)(PriorityQueue*);
	size_t (*length)(PriorityQueue*);
	bool (*empty)(PriorityQueue*);
	void (*push)(PriorityQueue*, const void*);
	void (*change)(PriorityQueue*, size_t, const void*);
	const void* (*erase)(PriorityQueue*, size_t);
	const void* (*pop)(PriorityQueue*);
	const void* (*get_top)(PriorityQueue*);
	void (*for_each)(PriorityQueue*, SequenceForEachFunc_Const, void*);
}PriorityQueueOp;

extern const PriorityQueueOp* GetPriorityQueueOpStruct();
#define PriorityQueue() (*(GetPriorityQueueOpStruct()))
#endif