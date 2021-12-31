#ifndef _BP_TREE__
#define _BP_TREE__
#include "../common/common.h"
#include <stdbool.h>

typedef unsigned short __keynode_size_t;
typedef unsigned short __value_size_t;
typedef struct BPMetaNode{
	unsigned long long rows;//实际数据行数
	off_t rootPointer;//根结点指针,初始为-1
	off_t firstPointer;//第一个叶子结点指针,初始为-1
	off_t fileSize;//元文件大小
	size_t keySize;//键大小
	size_t valSize;//值大小
	size_t indexBitMapEdge;//位图字节分界,前面为索引文件位图,后面为数据文件位图
	int metaPageSize;//meta页大小
	int indexPageSize;//索引页大小
	int dataPageSize;//数据页大小
	int dataPageBitBytes;//数据页位图字节数
	__value_size_t maxDC;//数据页最大行数
	__keynode_size_t maxNC;//结点最大关键字数
	__keynode_size_t minNC;//结点最小关键字数
}BPMetaNode;

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
	char META_FILENAME[4096];
	char INDEX_FILENAME[4096];
	char DATA_FILENAME[4096];
	BPMetaNode meta;
	BPKeyCompareFuncT equalFunc;
	BPKeyCompareFuncT lessFunc;
	void* tmpRet;
	char* tmpWriteStr;
	char* metaMap;//元文件映射
	int fdMeta;
	int fdIndex;
	int fdData;
}BPTree;

typedef struct BPTreeOp{
	BPTree* (*create)(size_t, size_t, BPKeyCompareFuncT, BPKeyCompareFuncT, const char*);
	void (*destroy)(BPTree**);
	void (*traverse)(BPTree*, BPForEachFuncT);
	void (*insert)(BPTree*, const void*, const void*);
	void (*erase)(BPTree*, const void*);
	const void* (*at)(BPTree*, const void*);
	void (*change)(BPTree*, const void*, const void*);
	unsigned long long (*rows)(BPTree*);
	BPTree* (*rebuild)(BPTree**);
}BPTreeOp;

extern const BPTreeOp* GetBPTreeOpStruct();
#define BPTree() (*(GetBPTreeOpStruct()))
#endif