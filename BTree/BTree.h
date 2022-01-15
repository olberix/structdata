#ifndef _B_TREE__
#define _B_TREE__
#include "../common/common.h"
#include <stdbool.h>

typedef struct HeaderNode{
	off_t rootPointer;
	size_t keySize;
	size_t valSize;
	int pageSize;
}HeaderNode;

typedef struct BNode{
	off_t* childPointers;
	void* pKey;
	void* pValue;
	off_t selfPointer;//标记这个node在文件的偏移位置,为0时代表新插入的node,还没有写入文件
	ssize_t size;
}BNode;

typedef struct BTree{
	HeaderNode head;
	CmnCompareFunc equalFunc;
	CmnCompareFunc lessFunc;
	void* tmpRet;
	int fd;
	unsigned short maxNC;
	unsigned short minNC;
}BTree;

typedef struct BTreeOp{
	BTree* (*create)(size_t, size_t, CmnCompareFunc, CmnCompareFunc, const char*);
	void (*destroy)(BTree**);
	void (*level_order_traverse)(BTree*, UnorderedForEachFunc_Const, void*);
	void (*traverse)(BTree*, UnorderedForEachFunc_Const, void*);
	void (*insert)(BTree*, const void*, const void*);
	void (*erase)(BTree*, const void*);
	const void* (*at)(BTree*, const void*);
	void (*change)(BTree*, const void*, const void*);
}BTreeOp;

extern const BTreeOp* GetBTreeOpStruct();
#define BTree() (*(GetBTreeOpStruct()))
#endif