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
	void (*for_each)(DlQueue*, DucLFEFuncT);
}DlQueueOp;

extern const DlQueueOp* GetDlQueueOpStruct();
#define DlQueue() (*(GetDlQueueOpStruct()))

#endif