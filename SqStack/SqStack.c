#include "SqStack.h"

static SqStack* create(size_t ESize, const size_t* pLSize)
{
	POINTCREATE(SqStack*, ret, SqStack, sizeof(SqStack));
	ret->list = SqList().create(ESize, pLSize);
	return ret;
}

static const void* get_top(SqStack* stack)
{
	SqList* pList = stack->list;
	size_t length = SqList().length(pList);
	CONDCHECK(length > 0, STATUS_NOELEM);
	return SqList().at(pList, length - 1);
}

static const void* pop(SqStack* stack)
{
	SqList* pList = stack->list;
	size_t length = SqList().length(pList);
	CONDCHECK(length > 0, STATUS_NOELEM);
	return SqList().erase(pList, length - 1);
}

static inline void push(SqStack* stack, const void* elem)
{
	SqList().push_back(stack->list, elem);
}

static inline bool empty(SqStack* stack)
{
	return SqList().length(stack->list) == 0;
}

static inline void clear(SqStack* stack)
{
	SqList().clear(stack->list);
}

static inline void destroy(SqStack** stack)
{
	SqList().destroy(&((*stack)->list));
	FREE((*stack));
}

static inline void for_each(SqStack* stack, SqlFEFuncT func)
{
	SqList().for_each(stack->list, func);
}

inline const SqStackOp* GetSqStackOpStruct()
{
	static const SqStackOp OpList = {
		.create = create,
		.clear = clear,
		.destroy = destroy,
		.get_top = get_top,
		.pop = pop,
		.push = push,
		.empty = empty,
		.for_each = for_each,
	};
	return &OpList;
}