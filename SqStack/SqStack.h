#ifndef _SQ_STACK__
#define _SQ_STACK__
#include "../common.h"
#include "../SqList/SqList.h"

typedef struct SqStack{
	SqList* list;
}SqStack;

typedef struct SqStackOp{
	SqStack* (*create)(size_t, const size_t*);
	void (*clear)(SqStack*);
	void (*destroy)(SqStack**);
	const void* (*get_top)(SqStack*);
	const void* (*pop)(SqStack*);
	void (*push)(SqStack*, const void*);
	bool (*empty)(SqStack*);
	void (*for_each)(SqStack*, SqlFEFuncT);
}SqStackOp;

extern const SqStackOp* GetSqStackOpStruct();
#define SqStack() (*(GetSqStackOpStruct()))
#endif