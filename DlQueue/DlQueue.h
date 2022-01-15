#ifndef _DLQUEUE__
#define _DLQUEUE__
#include "../DuCirLinkList/DuCirLinkList.h"

typedef struct DlQueue{
	DuCirLinkList* list;
}DlQueue;

typedef struct DlQueueOp{
	DlQueue* (*create)(size_t);
	void (*clear)(DlQueue*);
	void (*destroy)(DlQueue**);
	const void* (*pop)(DlQueue*);
	void (*push)(DlQueue*, const void*);
	bool (*empty)(DlQueue*);
	void (*for_each)(DlQueue*, SequenceForEachFunc_Mutable, void*);
	const void* (*at)(DlQueue*, size_t);
}DlQueueOp;

#define DLQUEUE_FOREACH(queue, type, logic) DULIST_FOREACH((queue)->list, type, logic)
#define DLQUEUE_FOREACH_REVERSE(queue, type, logic) DULIST_FOREACH_REVERSE((queue)->list, type, logic)

extern const DlQueueOp* GetDlQueueOpStruct();
#define DlQueue() (*(GetDlQueueOpStruct()))
#endif