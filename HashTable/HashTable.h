#ifndef _HASH_TABLE__
#define _HASH_TABLE__
#include "../common/common.h"
#include "../SqList/SqList.h"
#include "../RBTree/RBTree.h"
#include "../DuCirLinkList/DuCirLinkList.h"

static const unsigned char BUCKETTYPE_NIL = 0x0;
static const unsigned char BUCKETTYPE_ORIGIN = 0x1;
static const unsigned char BUCKETTYPE_LIST = 0x2;
static const unsigned char BUCKETTYPE_RBTREE = 0x3;

static const size_t HASHINITSIZE = 8;//哈希表初始长度
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

typedef size_t(*HashCodeFuncT)(const unsigned char*, size_t);
typedef bool(*HKeyCompareFuncT)(const void*, const void*);
typedef void(*HashForEachFuncT)(const void*, void*);
typedef struct HashTable{
	SqList* list;
	void* tmpRet;
	HashCodeFuncT hashFunc;
	HKeyCompareFuncT lessFunc;
	HKeyCompareFuncT equalFunc;
	size_t keySize;
	size_t valSize;
	size_t table_size;
	size_t elem_count;
}HashTable;

typedef struct HashTableOp{
	HashTable* (*create)(size_t, size_t, HashCodeFuncT, HKeyCompareFuncT, HKeyCompareFuncT);
	void (*clear)(HashTable*);
	void (*destroy)(HashTable**);
	void (*for_each)(HashTable*, HashForEachFuncT);
	void (*insert)(HashTable*, const void*, const void*);
	void (*erase)(HashTable*, const void*);
	const void* (*at)(HashTable*, const void*);
	void (*change)(HashTable*, const void*, const void*);
	size_t (*size)(HashTable*);
}HashTableOp;

extern const HashTableOp* GetHashTableOpStruct();
#define HashTable() (*(GetHashTableOpStruct()))

#endif