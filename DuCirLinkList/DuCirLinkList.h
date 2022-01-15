#ifndef _DUCIRLINKLIST__
#define _DUCIRLINKLIST__
#include "../common/common.h"
#include <stdbool.h>

typedef struct DuCirLink{
	void* pElem;
	struct DuCirLink* prior;
	struct DuCirLink* next;
}DuCirLink;
typedef struct DuCirLinkList{
	DuCirLink* link;
	void* tmpRet;
	size_t e_S;
	size_t length;
}DuCirLinkList;

#define BEGIN(DL) ((DL)->link->next)
#define LAST(DL) ((DL)->link->prior)
#define HEAD(DL) ((DL)->link)
#define END(DL) HEAD(DL)

typedef struct DucListOp{
	DuCirLinkList* (*create)(size_t);
	void (*destroy)(DuCirLinkList**);
	void (*clear)(DuCirLinkList*);
	void (*insert)(DuCirLinkList*, size_t, const void*);
	void (*push_back)(DuCirLinkList*, const void*);
	void (*change)(DuCirLinkList*, size_t, const void*);
	const void* (*erase)(DuCirLinkList*, size_t);
	const void* (*at)(DuCirLinkList*, size_t);
	size_t (*length)(DuCirLinkList*);
	void (*for_each)(DuCirLinkList*, SequenceForEachFunc_Mutable, void*);
	void (*r_for_each)(DuCirLinkList*, SequenceForEachFunc_Mutable, void*);
	void (*reverse)(DuCirLinkList*);
	void (*sort)(DuCirLinkList*, CmnCompareFunc);
}DucListOp;

#define DULIST_FOREACH(pList, type, logic) {\
	DuCirLink* _NODE__ = BEGIN(pList);\
	for (size_t key = 0; _NODE__ != END(pList); key++){\
		type value = TOCONSTANT(type, _NODE__->pElem);\
		logic;\
		_NODE__ = _NODE__->next;\
	}\
}

#define DULIST_FOREACH_REVERSE(pList, type, logic) {\
	DuCirLink* _NODE__ = LAST(pList);\
	for (size_t key = pList->length - 1; _NODE__ != END(pList); key--){\
		type value = TOCONSTANT(type, _NODE__->pElem);\
		logic;\
		_NODE__ = _NODE__->prior;\
	}\
}

extern const DucListOp* GetDucListOpStruct();
#define DucList() (*(GetDucListOpStruct()))
#endif