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
	list->equalFunc = equalFunc;
	list->lessFunc = lessFunc;
	return list;
}

static inline void clear(SkipList* list)
{
	if (list->length == 0)
		return;
	for(SkipListNode* node = SKL_LAST(list); node != SKL_HEAD(list);){
		SkipListNode* tmpNode = node;
		node = node->backward;
		FREE(tmpNode->pKey);
		FREE(tmpNode);
	}
	memset(SKL_HEAD(list)->levelInfo, 0, sizeof(struct SkipListLevel) * list->level);
	SKL_BEGIN(list) = SKL_HEAD(list);
	SKL_LAST(list) = SKL_END(list);
	list->length = 0;
	list->level = 0;
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
	while((1.0f * rand()) / RAND_MAX <= SKIPLIST_P && level < SKIPLIST_MAXLEVEL)
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
	struct SkipListLevel markPoints[SKIPLIST_MAXLEVEL];
	for(int i = 0; i < SKIPLIST_MAXLEVEL; i++){
		markPoints[i].forward = SKL_HEAD(list);
		markPoints[i].span = 0;
	}

	SkipListNode* foreNode = SKL_HEAD(list);
	size_t curLoc = 0;
	for (int i = list->level - 1; i >= 0; i--){
		do{
			SkipListNode* tmp = foreNode->levelInfo[i].forward;
			if (tmp != SKL_END(list)){
				if (list->equalFunc(tmp->pKey, pKey))
					return;
				if (list->lessFunc(tmp->pKey, pKey))
					break;
				foreNode = tmp;
				curLoc += foreNode->levelInfo[i].span;
			}
			else
				break;
		}while(true);
		markPoints[i].forward = foreNode;
		markPoints[i].span = curLoc;
	}

	curLoc++;
	SkipListNode* newNode = __gen_new_node(list, pKey);
	foreNode->levelInfo[0].forward->backward = newNode;
	newNode->backward = foreNode;
	for(int i = newNode->level - 1; i >= 0; i--){
		struct SkipListLevel* _fl = markPoints[i].forward->levelInfo + i;
		if (_fl->forward){
			newNode->levelInfo[i].forward = _fl->forward;
			if (_fl->forward != SKL_END(list))
				_fl->forward->levelInfo[i].span += (markPoints[i].span + 1 - curLoc);
		}
		else
			newNode->levelInfo[i].forward = SKL_END(list);
		newNode->levelInfo[i].span = curLoc - markPoints[i].span;
		_fl->forward = newNode;
	}
	for(int i = newNode->level; i < list->level; i++){
		struct SkipListLevel* _fl = markPoints[i].forward->levelInfo + i;
		if (_fl->forward && _fl->forward != SKL_END(list))
			_fl->forward->levelInfo[i].span++;
	}
	if (newNode->level > list->level)
		list->level = newNode->level;
	list->length++;
}

static void erase(SkipList* list, const void* pKey)
{
	SkipListNode* markNodes[SKIPLIST_MAXLEVEL];
	SkipListNode* node = SKL_HEAD(list);
	for (int i = list->level - 1; i >= 0; i--){
		while(true){
			SkipListNode* tmp = node->levelInfo[i].forward;
			if (tmp != SKL_END(list)){
				if (list->equalFunc(tmp->pKey, pKey)){
					markNodes[i] = node;
					node = tmp;
					for (i--; i >= 0; i--){
						SkipListNode* _n = markNodes[i + 1];
						while(_n->levelInfo[i].forward != node)
							_n = _n->levelInfo[i].forward;
						markNodes[i] = _n;
					}
					break;
				}
				if (list->lessFunc(tmp->pKey, pKey)){
					markNodes[i] = node;
					break;
				}
				node = tmp;
			}
			else{
				markNodes[i] = node;
				break;
			}
		}
	}
	if (node == SKL_END(list) || !list->equalFunc(node->pKey, pKey))
		return;

	node->levelInfo[0].forward->backward = node->backward;
	for(int i = node->level - 1; i >= 0; i--){
		SkipListNode* _ff = node->levelInfo[i].forward;
		if (markNodes[i] == SKL_HEAD(list) && _ff == SKL_END(list)){
			markNodes[i]->levelInfo[i].forward = NULL;
			list->level--;
		}
		else
			markNodes[i]->levelInfo[i].forward = _ff;
		if (_ff != SKL_END(list))
			_ff->levelInfo[i].span += (node->levelInfo[i].span - 1);
	}
	for(int i = node->level; i < list->level; i++){
		SkipListNode* _ff = markNodes[i]->levelInfo[i].forward;
		if (_ff != SKL_END(list))
			_ff->levelInfo[i].span--;
	}
	list->length--;

	FREE(node->pKey);
	FREE(node);
}

static void erase_loc(SkipList* list, size_t loc)
{
	if (loc >= list->length)
		return;

	loc++;
	SkipListNode* markNodes[SKIPLIST_MAXLEVEL];
	SkipListNode* node = SKL_HEAD(list);
	size_t curLoc = 0;
	for (int i = list->level - 1; i >= 0; i--){
		if (curLoc == loc){
			SkipListNode* _n = markNodes[i + 1];
			while(_n->levelInfo[i].forward != node)
				_n = _n->levelInfo[i].forward;
			markNodes[i] = _n;
		}
		else
			while(true){
				SkipListNode* tmp = node->levelInfo[i].forward;
				if (tmp != SKL_END(list)){
					size_t tmpLoc = curLoc + tmp->levelInfo[i].span;
					if (tmpLoc > loc){
						markNodes[i] = node;
						break;
					}
					markNodes[i] = node;
					node = tmp;
					curLoc = tmpLoc;
					if (curLoc == loc)
						break;
				}
				else{
					markNodes[i] = node;
					break;
				}
			}
	}

	node->levelInfo[0].forward->backward = node->backward;
	for(int i = node->level - 1; i >= 0; i--){
		SkipListNode* _ff = node->levelInfo[i].forward;
		if (markNodes[i] == SKL_HEAD(list) && _ff == SKL_END(list)){
			markNodes[i]->levelInfo[i].forward = NULL;
			list->level--;
		}
		else
			markNodes[i]->levelInfo[i].forward = _ff;
		if (_ff != SKL_END(list))
			_ff->levelInfo[i].span += (node->levelInfo[i].span - 1);
	}
	for(int i = node->level; i < list->level; i++){
		SkipListNode* _ff = markNodes[i]->levelInfo[i].forward;
		if (_ff != SKL_END(list))
			_ff->levelInfo[i].span--;
	}
	list->length--;

	FREE(node->pKey);
	FREE(node);
}

static const void* at(SkipList* list, size_t loc)
{
	if (loc >= list->length)
		return NULL;

	loc++;
	SkipListNode* node = SKL_HEAD(list);
	size_t curLoc = 0;
	for (int i = list->level - 1; i >= 0; i--){
		do{
			SkipListNode* tmp = node->levelInfo[i].forward;
			if (tmp != SKL_END(list)){
				size_t tmpLoc = curLoc + tmp->levelInfo[i].span;
				if (tmpLoc == loc)
					return tmp->pKey;
				else if (tmpLoc > loc)
					break;
				else{
					node = tmp;
					curLoc = tmpLoc;
				}
			}
			else
				break;
		}while(true);
	}

	return NULL;
}

static long long find(SkipList* list, const void* pKey)
{
	SkipListNode* node = SKL_HEAD(list);
	size_t loc = 0;
	for(int i = list->level - 1; i >= 0; i--){
		do{
			SkipListNode* tmp = node->levelInfo[i].forward;
			if (tmp != SKL_END(list)){
				if (list->equalFunc(tmp->pKey, pKey))
					return loc + tmp->levelInfo[i].span - 1;
				if (list->lessFunc(tmp->pKey, pKey))
					break;
				node = tmp;
				loc += tmp->levelInfo[i].span;
			}
			else
				break;
		}while(true);
	}

	return -1;
}

static inline size_t length(SkipList* list)
{
	return list->length;
}

static void for_each(SkipList* list, SequenceForEachFunc_Set func, void* args)
{
	if (list->length == 0)
		return;
	size_t index = 0;
	for(SkipListNode* node = SKL_BEGIN(list); node != SKL_END(list); node = node->levelInfo[0].forward, index++)
		func(index, node->pKey, args);
}

static void r_for_each(SkipList* list, SequenceForEachFunc_Set func, void* args)
{
	if (list->length == 0)
		return;
	size_t index = list->length - 1;
	for(SkipListNode* node = SKL_LAST(list); node != SKL_HEAD(list); node = node->backward, index--)
		func(index, node->pKey, args);
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