#include "SkipList.h"
#include <time.h>

static SkipList* create(size_t keySize, CmnCompareFunc equalFunc, CmnCompareFunc lessFunc)
{
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	POINTCREATE_INIT(SkipList*, list, SkipList, sizeof(SkipList));
	POINTCREATE_INIT(EMPTYDEF, list->header, void, sizeof(SkipListNode) + sizeof(struct SkipListLevel) * SKIPLIST_MAXLEVEL);
	SKL_BEGIN(list) = SKL_HEAD(list);
	SKL_LAST(list) = SKL_END(list);
	list->keySize = keySize;
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

static inline unsigned char __random_level()
{
	unsigned char level = 1;
	while(1.0f * rand() / RAND_MAX <= SKIPLIST_P && level < SKIPLIST_MAXLEVEL)
		level++;
	return level;
}

static inline SkipListNode* __gen_new_node(SkipList* list, const void* pKey)
{
	unsigned char nodeLevel = __random_level();
	POINTCREATE(SkipListNode*, node, SkipListNode, sizeof(SkipListNode) + sizeof(struct SkipListLevel) * nodeLevel);
	POINTCREATE(EMPTYDEF, node->pKey, void, list->keySize);
	memcpy(node->pKey, pKey, list->keySize);
	node->level = nodeLevel;
	return node;
}

static void insert(SkipList* list, const void* pKey)
{
	SkipListNode* markPoints[SKIPLIST_MAXLEVEL];
	for(int i = 0; i < SKIPLIST_MAXLEVEL; i++)
		markPoints[i] = SKL_HEAD(list);

	SkipListNode* foreNode = SKL_HEAD(list);
	for (int i = list->level - 1; i >= 0; i--){
		do{
			SkipListNode* tmp = foreNode->levelInfo[i].forward;
			if (tmp != SKL_END(L)){
				if (list->equalFunc(tmp->pKey, pKey))
					return;
				if (list->lessFunc(tmp->pKey, pKey))
					break;
				foreNode = tmp;
			}
		}while(true);
		markPoints[i] = foreNode;
	}

	SkipListNode* newNode = __gen_new_node(list, pKey);
	
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