#include "SqList.h"
#include <string.h>

static SqList* create(size_t ESize, const size_t* pLSize)
{
	CONDCHECK(ESize > 0, STATUS_SIZEERROR);
	POINTCREATE(SqList*, ret, SqList, sizeof(SqList));
	size_t _size = INIT_SIZE;
	if (pLSize)
		if (*pLSize > _size)
			_size = *pLSize;
	POINTCREATE(EMPTYDEF, ret->pElems, void, ESize * _size);
	POINTCREATE(EMPTYDEF, ret->tmpRet, void, ESize);
	ret->e_S = ESize;
	ret->size = _size;
	ret->length = 0;
	return ret;
}

static void inline destroy(SqList** ppList)
{
	FREE((*ppList)->pElems);
	FREE((*ppList)->tmpRet);
	FREE(*ppList);
}

static void inline clear(SqList* pList)
{
	pList->length = 0;
}

static inline void CHECK_REALLOC(SqList* pList)
{
	if (pList->length < pList->size)
		return;
	size_t _size = pList->size * INCREASE_RATE;
	pList->pElems = (void*)realloc(pList->pElems, pList->e_S * _size);
	CONDCHECK(pList->pElems, STATUS_OVERFLOW);
	pList->size = _size;
}

static void insert(SqList* pList, size_t loc, const void* elem)
{
	CONDCHECK(loc >= 0 && loc <= pList->length, STATUS_INVALIDINDEX);
	CHECK_REALLOC(pList);
	size_t e_S = pList->e_S;
	void* locAddr = pList->pElems + loc * e_S;
	memmove(locAddr + e_S, locAddr, e_S * (pList->length - loc));
	memcpy(locAddr, elem, e_S);
	pList->length++;
}

static inline void push_back(SqList* pList, const void* elem)
{
	insert(pList, pList->length, elem);
}

static inline void change(SqList* pList, size_t loc, const void* elem)
{
	CONDCHECK(loc >= 0 && loc < pList->length, STATUS_INVALIDINDEX);
	memcpy(pList->pElems + loc * pList->e_S, elem, pList->e_S);
}

static const void* erase(SqList* pList, size_t loc)
{
	CONDCHECK(loc >= 0 && loc < pList->length, STATUS_INVALIDINDEX);
	void* locAddr = pList->pElems + loc * pList->e_S;
	memcpy(pList->tmpRet, locAddr, pList->e_S);
	memmove(locAddr, locAddr + pList->e_S, pList->e_S * (pList->length - 1 - loc));
	pList->length--;
	return pList->tmpRet;
}

static const inline void* at(SqList* pList, size_t loc)
{
	CONDCHECK(loc >= 0 && loc < pList->length, STATUS_INVALIDINDEX);
	return pList->pElems + loc * pList->e_S;
}

static inline size_t length(SqList* pList)
{
	return pList->length;
}

static void reverse(SqList* pList)
{
	if (pList->length <= 1)
		return;
	size_t e_S =  pList->e_S;
	POINTCREATE(void*, tmpPoint, void, e_S);
	void* left = pList->pElems;
	void* right = pList->pElems + e_S * (pList->length - 1);
	for (size_t i = 0; i < pList->length / 2; i++){
		memcpy(tmpPoint, left, e_S);
		memcpy(left, right, e_S);
		memcpy(right, tmpPoint, e_S);
		left += e_S;
		right -= e_S;
	}
	FREE(tmpPoint);
}

static inline void for_each(SqList* pList, SqlFEFuncT func)
{
	CONDCHECK(pList->length != 0, STATUS_NOELEM);
	for (size_t i = 0; i < pList->length; i++)
		func(i, pList->pElems + i * pList->e_S);
}

static inline void r_for_each(SqList* pList, SqlFEFuncT func)
{
	CONDCHECK(pList->length != 0, STATUS_NOELEM);
	for (size_t i = pList->length; i >= 1; i--)/*if i >= 0, unsigned arith will overflow*/
		func(i - 1, pList->pElems + (i - 1) * pList->e_S);
}

/*C11 std include qsort*/
static void QUICK_SORT(SqList* pList, SqlCmpFuncT func, size_t left, size_t right)
{
	size_t e_S = pList->e_S;
	POINTCREATE(void*, pivot, void, e_S);
	void* pElems = pList->pElems;
	size_t _ll = left, _rr = right;
	memcpy(pivot, pElems + left * e_S, e_S);
	while (_ll < _rr){
		while (_ll < _rr && !func(pElems + _rr * e_S, pivot))
			_rr--;
		if (_ll < _rr)
			memcpy(pElems + (_ll++) * e_S, pElems + _rr * e_S, e_S);
		while (_ll < _rr && func(pElems + _ll * e_S, pivot))
			_ll++;
		if (_ll < _rr)
			memcpy(pElems + (_rr--) * e_S, pElems + _ll * e_S, e_S);
	}
	memcpy(pElems + _ll * e_S, pivot, e_S);
	if (_ll - left > 2)
		QUICK_SORT(pList, func, left, _ll - 1);
	if (right - _ll > 2)
		QUICK_SORT(pList, func, _ll + 1, right);
}

static inline void sort(SqList* pList, SqlCmpFuncT func)
{
	if (pList->length <= 1)
		return;
	QUICK_SORT(pList, func, 0, pList->length - 1);
}

const inline SqListOp* GetSqListOpStruct()
{
	static const SqListOp OpList = {
		.create = create,
		.destroy = destroy,
		.clear = clear,
		.insert = insert,
		.push_back = push_back,
		.change = change,
		.erase = erase,
		.at = at,
		.length = length,
		.for_each = for_each,
		.r_for_each = r_for_each,
		.reverse = reverse,
		.sort = sort,
	};
	return &OpList;
}