#ifndef _HASH_TABLE__
#define _HASH_TABLE__
#include "../common/common.h"
#include "../SqList/SqList.h"
#include "../RBTree/RBTree.h"
#include "../DuCirLinkList/DuCirLinkList.h"

#define BUCKETTYPE_NIL 0x0
#define BUCKETTYPE_ORIGIN 0x1
#define BUCKETTYPE_LIST 0x2
#define BUCKETTYPE_RBTREE 0x3
#define LOADFACTOR 0.75f

static const size_t HASHINITSIZE = 16;//哈希表初始长度
static const size_t TRANSTOTREESIZE = 6;//链表转化为红黑树的元素个数

typedef struct HashEntry{
	void* pKey;
	void* pValue;
}HashEntry;

typedef union __HashBucket{
	HashEntry entry;
	DuCirLinkList* entry_list;
	RBTree* entry_tree;
}__HashBucket;

typedef struct HashBucket{
	__HashBucket bucket;
	unsigned char type;
}HashBucket;

typedef struct HashTable{
	SqList* list;
	void* tmpRet;
	HashFuncTT hashFunc;
	CmnCompareFunc lessFunc;
	CmnCompareFunc equalFunc;
	size_t keySize;
	size_t valSize;
	size_t table_size;
	size_t maxLoadFactor;
	size_t elem_count;
}HashTable;

typedef struct HashTableOp{
	HashTable* (*create)(size_t, size_t, HashFuncTT, CmnCompareFunc, CmnCompareFunc);
	void (*clear)(HashTable*);
	void (*destroy)(HashTable**);
	void (*for_each)(HashTable*, UnorderedForEachFunc_Mutable, void*);
	void (*insert)(HashTable*, const void*, const void*);
	const void* (*erase)(HashTable*, const void*);
	const void* (*at)(HashTable*, const void*);
	void (*change)(HashTable*, const void*, const void*);
	size_t (*size)(HashTable*);
}HashTableOp;

extern const HashTableOp* GetHashTableOpStruct();
#define HashTable() (*(GetHashTableOpStruct()))

#endif