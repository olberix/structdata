#ifndef _DUCIRLINKLIST__
#define _DUCIRLINKLIST__
#include "../common.h"
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

typedef void(*DucLFEFuncT)(size_t, void*);
typedef bool(*DucLCmpFuncT)(const void*, const void*);
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
	void (*for_each)(DuCirLinkList*, DucLFEFuncT);
	void (*r_for_each)(DuCirLinkList*, DucLFEFuncT);
	void (*reverse)(DuCirLinkList*);
	void (*sort)(DuCirLinkList*, DucLCmpFuncT);
}DucListOp;

#define DULIST_FOREACH(pList, type, logic) {\
	DuCirLink* _NODE__ = BEGIN(pList);\
	for (size_t _INDEX__ = 0; _NODE__ != END(pList); _INDEX__++){\
		DuCirLink* tmpNode = _NODE__->next;\
		type* _PVALUE__ = (type*)(_NODE__->pElem);\
		logic;\
		_NODE__ = tmpNode;\
	}\
}

#define DULIST_FOREACH_REVERSE(pList, type, logic) {\
	DuCirLink* _NODE__ = LAST(pList);\
	for (size_t _INDEX__ = pList->length - 1; _NODE__ != END(pList); _INDEX__--){\
		DuCirLink* tmpNode = _NODE__->prior;\
		type* _PVALUE__ = (type*)(_NODE__->pElem);\
		logic;\
		_NODE__ = tmpNode;\
	}\
}

extern const DucListOp* GetDucListOpStruct();
#define DucList() (*(GetDucListOpStruct()))
#endif