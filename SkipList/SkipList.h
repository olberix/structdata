#ifndef _SKIP_LIST__
#define _SKIP_LIST__
#include "../common/common.h"

#define SKIPLIST_MAXLEVEL 32
#define SKIPLIST_P 0.25f

#define SKL_BEGIN(L) (L)->header->levelInfo[0].forward
#define SKL_LAST(L) (L)->header->backward
#define SKL_HEAD(L) (L)->header
#define SKL_END(L) SKL_HEAD(L)

typedef struct SkipListNode{
	void* pKey;
	struct SkipListNode* backward;
	unsigned short level;
	struct SkipListLevel{
		struct SkipListNode* forward;
		size_t span;
	}levelInfo[];
}SkipListNode;

typedef struct SkipList{
	SkipListNode* header;
	size_t length;
	size_t keySize;
	unsigned short level;
}SkipList;

typedef struct SkipListOp{
	SkipList* (*create)(size_t, CmnCompareFunc, CmnCompareFunc);
	void (*destroy)(SkipList**);
	void (*clear)(SkipList*);
	void (*insert)(SkipList*, const void*);
	void (*erase)(SkipList*, const void*);
	void (*erase_loc)(SkipList*, size_t);
	const void* (*at)(SkipList*, size_t);
	long long (*find)(SkipList*, const void*);
	size_t (*length)(SkipList*);
	void (*for_each)(SkipList*, SequenceForEachFunc_Set, void*);
	void (*r_for_each)(SkipList*, SequenceForEachFunc_Set, void*);
}SkipListOp;

extern const SkipListOp* GetSkipListOpStruct();
#define SkipList() (*(GetSkipListOpStruct()))
#endif