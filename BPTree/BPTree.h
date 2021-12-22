#ifndef _BP_TREE__
#define _BP_TREE__
#include "../common/common.h"
#include <stdbool.h>
#include <unistd.h>

typedef struct BPMetaNode{
	off_t rootPointer;//根结点指针
	off_t firstPointer;//第一个叶子结点指针
	off_t fileSize;//元文件大小
	size_t keySize;//键大小
	size_t valSize;//值大小
	size_t indexBMEnd;//索引地址位图结束
	int metaPageSize;//元文件地址位图分配单位,等于默认内存页大小
	int indexPageSize;//索引文件结点分配单位,等于默认内存页大小*PAGE_RATE
}BPMetaNode;

typedef struct BNode{
	void* pKey;
	off_t* childPointers;//分配空间size+1,最后一个为next指针
	off_t selfPointer;
	BNodeST size;
	unsigned char isLeaf;
}BNode;

typedef bool(*BPKeyCompareFuncT)(const void*, const void*);
typedef void(*BPForEachFuncT)(const void*, const void*);
typedef struct BPTree{
	BPMetaNode meta;
	BPKeyCompareFuncT equalFunc;
	BPKeyCompareFuncT lessFunc;
	void* tmpRet;
	char* pMeta;//元文件映射
	int fdMeta;
	int fdIndex;
	int fdData;
	unsigned short maxNC;
	unsigned short minNC;
}BPTree;

typedef struct BPTreeOp{
	BPTree* (*create)(size_t, size_t, BPKeyCompareFuncT, BPKeyCompareFuncT, const char*);
	void (*destroy)(BPTree**);
	void (*level_order_traverse)(BPTree*, BPForEachFuncT);
	void (*traverse)(BPTree*, BPForEachFuncT);
	void (*insert)(BPTree*, const void*, const void*);
	void (*erase)(BPTree*, const void*);
	const void* (*at)(BPTree*, const void*);
	void (*change)(BPTree*, const void*, const void*);
}BPTreeOp;

extern const BPTreeOp* GetBPTreeOpStruct();
#define BPTree() (*(GetBPTreeOpStruct()))
#endif