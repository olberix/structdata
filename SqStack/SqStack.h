#ifndef _SQ_STACK__
#define _SQ_STACK__
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
	void (*for_each)(SqStack*, SequenceForEachFunc_Mutable, void*);
	const void* (*at)(SqStack*, size_t);
}SqStackOp;

#define SQSTACK_FOREACH(stack, type, logic) SQLIST_FOREACH((stack)->list, type, logic)
#define SQSTACK_FOREACH_REVERSE(stack, type, logic) SQLIST_FOREACH_REVERSE((stack)->list, type, logic)

extern const SqStackOp* GetSqStackOpStruct();
#define SqStack() (*(GetSqStackOpStruct()))
#endif