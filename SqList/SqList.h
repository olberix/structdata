#ifndef _SQ_LIST__
#define _SQ_LIST__
#include "../common.h"
#include <stdbool.h>

#define INIT_SIZE 150
#define INCREASE_RATE 1.75

typedef struct SqList{
	void* pElems;
	void* tmpRet;
	size_t e_S;
	size_t length;
	size_t size;
}SqList;

typedef void(*SqlFEFuncT)(size_t, void*);
typedef bool(*SqlCmpFuncT)(const void*, const void*);
typedef struct SqListOp{
	SqList* (*create)(size_t, const size_t*);
	void (*destroy)(SqList**);
	void (*clear)(SqList*);
	void (*insert)(SqList*, size_t, const void*);
	void (*push_back)(SqList*, const void*);
	void (*change)(SqList*, size_t, const void*);
	const void* (*erase)(SqList*, size_t);
	const void* (*at)(SqList*, size_t);
	size_t (*length)(SqList*);
	void (*for_each)(SqList*, SqlFEFuncT);
	void (*r_for_each)(SqList*, SqlFEFuncT);/*forward*/
	void (*reverse)(SqList*);
	void (*sort)(SqList*, SqlCmpFuncT);
}SqListOp;

/*
do SQLIST_FOREACH and SQLIST_FOREACH_REVERSE except insert and push_back because the address of the pElems will change
of cause you can cal _PVALUE__ at every times but that maybe senseless to do insert and push_back?
be careful about the _INDEX__ and _PVALUE__ while doing erase
*/
#define SQLIST_FOREACH(pList, type, logic) {\
	type* _PVALUE__ = (type*)(pList)->pElems;\
	for (size_t _INDEX__ = 0; _INDEX__ < (pList)->length; _INDEX__++, _PVALUE__++){\
		logic;\
	}\
}

#define SQLIST_FOREACH_REVERSE(pList, type, logic) {\
	type* _PVALUE__ = (type*)(pList)->pElems + (pList)->length - 1;\
	for (size_t tmpIndex = (pList)->length; tmpIndex >= 1; tmpIndex--, _PVALUE__--){\
		size_t _INDEX__ = tmpIndex - 1;\
		logic;\
	}\
}

extern const SqListOp* GetSqListOpStruct();
#define SqList() (*(GetSqListOpStruct()))
#endif