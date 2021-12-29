#ifndef _BP_TREE__
#define _BP_TREE__
#include "../common/common.h"
#include <stdbool.h>

typedef struct BPMetaNode{
	off_t rootPointer;//根结点指针,这个初始为-1
	off_t firstPointer;//第一个叶子结点指针,这个初始为-1
	off_t fileSize;//元文件大小
	size_t keySize;//键大小
	size_t valSize;//值大小
	size_t indexBitMapEdge;//索引地址位图结束
	int metaPageSize;//元文件地址位图分配单位,等于默认内存页大小
	int indexPageSize;//索引文件结点分配单位,等于默认内存页大小*PAGE_RATE
}BPMetaNode;

typedef unsigned short __keynode_size_t;
typedef struct BPNode{
	void* pKey;
	off_t* childPointers;//分配空间size+1,最后一个为next指针
	off_t selfPointer;//与B树不同,这个初始为-1
	__keynode_size_t size;//结点关键字数
	unsigned char isLeaf;//0为内部结点,1为叶子结点
}BPNode;

typedef bool(*BPKeyCompareFuncT)(const void*, const void*);
typedef void(*BPForEachFuncT)(const void*, const void*);
typedef struct BPTree{
	BPMetaNode meta;
	BPKeyCompareFuncT equalFunc;
	BPKeyCompareFuncT lessFunc;
	void* tmpRet;
	char* tmpWriteStr;
	char* metaMap;//元文件映射
	int fdMeta;
	int fdIndex;
	int fdData;
	__keynode_size_t maxNC;
	__keynode_size_t minNC;
}BPTree;

typedef struct BPTreeOp{
	BPTree* (*create)(size_t, size_t, BPKeyCompareFuncT, BPKeyCompareFuncT, const char*);
	void (*destroy)(BPTree**);
	void (*traverse)(BPTree*, BPForEachFuncT);
	void (*insert)(BPTree*, const void*, const void*);
	void (*erase)(BPTree*, const void*);
	const void* (*at)(BPTree*, const void*);
	void (*change)(BPTree*, const void*, const void*);
}BPTreeOp;

extern const BPTreeOp* GetBPTreeOpStruct();
#define BPTree() (*(GetBPTreeOpStruct()))
#endif