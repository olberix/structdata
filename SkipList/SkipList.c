#include "SkipList.h"

static SkipList* create(size_t keySize, CmnCompareFunc equalFunc, CmnCompareFunc lessFunc)
{
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	POINTCREATE_INIT(SkipList*, list, SkipList, sizeof(SkipList));
	POINTCREATE_INIT(EMPTYDEF, list->header, void, sizeof(SkipListNode) + sizeof(struct SkipListLevel) * SKIPLIST_MAXLEVEL);
	SKL_BEGIN(list) = SKL_HEAD(list);
	SKL_LAST(list) = SKL_END(list);
	return list;
}

static inline void clear(SkipList* list)
{
	for(SkipListNode* node = SKL_LAST(list); node != SKL_HEAD(list);){
		SkipListNode* tmpNode = node;
		node = node->backward;
		FREE(tmpNode->pKey);
		FREE(tmpNode);
	}
	SKL_BEGIN(list) = SKL_HEAD(list);
	SKL_LAST(list) = SKL_END(list);
	list->length = 0;
}

static inline void destroy(SkipList** sList)
{
	clear(*sList);
	FREE(SKL_HEAD(*sList));
	FREE(*sList);
}

static void insert(SkipList* list, const void* pKey)
{
	SkipListNode* node = list->head;
	for (int i = list->level - 1; i >= 0; i--){
		do{
			SkipListNode* tmp = node->levelInfo[i].forward;
			if (tmp != SKL_HEAD(L)){
				if (list->equalFunc(tmp->pKey, pKey))
					return NULL;
				if (list->lessFunc(tmp->pKey, pKey))
					break;
				node = tmp;
			}
		}while(true);
	}
}

void erase(SkipList* list, const void* pKey)
{

}

void erase_loc(SkipList* list, size_t loc)
{

}

const void* at(SkipList* list, size_t loc)
{

}

long long find(SkipList* list, const void* pKey)
{

}

static inline size_t length(SkipList* list)
{
	return list->length;
}

void for_each(SkipList* list, SequenceForEachFunc_Set func, void* args)
{

}

void r_for_each(SkipList* list, SequenceForEachFunc_Set func, void* args)
{

}

inline const SkipListOp* GetSkipListOpStruct()
{
	static const SkipListOp OpList = {
		.create = create,
		.destroy = destroy,
		.clear = clear,
		.insert = insert,
		.erase = erase,
		.erase_loc = erase_loc,
		.at = at,
		.find = find,
		.length = length,
		.for_each = for_each,
		.r_for_each = r_for_each,
	};
	return &OpList;
}