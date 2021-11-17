#include "BTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "../DlQueue/DlQueue.h"

extern int fsync(int);
extern int getpagesize();
#define KEYSIZE (bt->head.keySize)
#define VALSIZE (bt->head.valSize)
#define PAGESIZE (bt->head.pageSize)

static BTree* create(size_t keySize, size_t valSize, BKeyCompareFuncT equalFunc, BKeyCompareFuncT lessFunc, const char* fileName)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC);
	POINTCREATE(BTree*, bt, BTree, sizeof(BTree));
	int flag = access(fileName, F_OK);
	bt->fd = open(fileName, O_RDWR | O_CREAT, 777);
	CONDCHECK(bt->fd > 0, STATUS_FDERROR);
	if (flag == 0){//文件存在,代表已经创建
		CONDCHECK(read(bt->fd, &(bt->head), sizeof(HeaderNode)) == sizeof(HeaderNode), STATUS_RDERROR);
		CONDCHECK(KEYSIZE == keySize && VALSIZE == valSize && PAGESIZE == getpagesize(), STATUS_SIZEERROR);
	}
	else{
		KEYSIZE = keySize;
		VALSIZE = valSize;
		PAGESIZE = getpagesize();
		POINTCREATE_INIT(char*, tmp, char, PAGESIZE);
		memcpy(tmp, &(bt->head), sizeof(HeaderNode));
		CONDCHECK(write(bt->fd, tmp, PAGESIZE) == PAGESIZE, STATUS_WRERROR);
	}
	POINTCREATE(EMPTYDEF, bt->tmpRet, void, valSize);
	bt->equalFunc = equalFunc;
	bt->lessFunc = lessFunc;
	bt->maxNC = (PAGESIZE - sizeof(off_t) - sizeof(size_t)) / (keySize + valSize + sizeof(off_t));
	CONDCHECK((bt->maxNC + 1) / 2 >= 2, STATUS_DEERROR);//最小度数t>=2
	bt->minNC = ((float)bt->maxNC + 1.5f) / 2.0f - 1;//结点合成:t-1+t-2+1=2t-2,遇到非整除情况t向上取整多出0.5,此时2(t+0.5)-2=2t-1刚好为最大结点数,达到最优结构
	return bt;//t往下取整也行,因为最小度数t>=2,所以2t-3恒大于t-1,同样满足B树性质
}

static inline void destroy(BTree** sbt)
{
	FREE((*sbt)->tmpRet);
	fsync((*sbt)->fd);
	close((*sbt)->fd);
	FREE(*sbt);
}

/*存储顺序:size-pKey-pValue-childPointers*/
static inline void READNODE(BTree* bt, off_t pointer, BNode* node)
{
	lseek(bt->fd, pointer, SEEK_SET);
	ssize_t _size = sizeof(node->size);
	CONDCHECK(read(bt->fd, &(node->size), _size) == _size, STATUS_RDERROR);
	_size = node->size * KEYSIZE;
	CONDCHECK(read(bt->fd, node->pKey, _size) == _size, STATUS_RDERROR);
	_size = node->size * VALSIZE;
	CONDCHECK(read(bt->fd, node->pValue, _size) == _size, STATUS_RDERROR);
	_size = (node->size + 1) * sizeof(off_t);
	memset(node->childPointers, 0, _size);//childPointers清0,判断是否叶子结点
	CONDCHECK(read(bt->fd, node->childPointers, _size) == _size, STATUS_RDERROR);
}

static inline bool ISLEAF(BNode* node)
{
	return node->childPointers[0] != 0;
}

static inline BNode* NEWBNODE(BTree* bt)
{
	POINTCREATE(BNode*, node, BNode, sizeof(BNode));
	POINTCREATE(EMPTYDEF, node->childPointers, off_t, sizeof(off_t) * (bt->maxNC + 1));
	POINTCREATE(EMPTYDEF, node->pKey, void, KEYSIZE * bt->maxNC);
	POINTCREATE(EMPTYDEF, node->pValue, void, VALSIZE * bt->maxNC);
	return node;
}

static inline void RELEASEBNODE(BNode** nnode)
{
	FREE((*nnode)->pValue);
	FREE((*nnode)->pKey);
	FREE((*nnode)->childPointers);
	FREE(*nnode);
}

static void level_order_traverse(BTree* bt, BForEachFuncT func)
{
	off_t rootPointer = bt->head.rootPointer;
	if (!rootPointer)
		return;
	DlQueue* queue = DlQueue().create(sizeof(off_t));
	DlQueue().push(queue, &rootPointer);
	BNode* node = NEWBNODE(bt);
	while(!DlQueue().empty(queue)){
		off_t pointer = TOCONSTANT(off_t, DlQueue().pop(queue));
		READNODE(bt, pointer, node);
		for (ssize_t i = 0; i < node->size + 1; i++)
			DlQueue().push(queue, node->childPointers + i);
		for (ssize_t i = 0; i < node->size; i++)
			func(node->pKey + KEYSIZE * i, node->pValue + VALSIZE * i);
	}
	RELEASEBNODE(&node);
}

/*理论需要最大栈空间S=log(minNC+1, N) * PAGESIZE,以minNC取极端值1为例,10亿级数据S约等于30,一般内存页大小为4K,此时需求内存为120K,洒洒水啦*/
static void __in_order_traverse(BTree* bt, off_t pointer, BForEachFuncT func)
{
	if (!pointer)
		return;
	BNode* node = NEWBNODE(bt);
	READNODE(bt, pointer, node);
	if (ISLEAF(node)){
		for(ssize_t i = 0; i < node->size; i++)
			func(node->pKey + i * KEYSIZE, node->pValue + i * VALSIZE);
		RELEASEBNODE(&node);
		return;
	}
	__in_order_traverse(bt, node->childPointers[0], func);
	for (ssize_t i = 0; i < node->size; i++){
		func(node->pKey + i * KEYSIZE, node->pValue + i * VALSIZE);
		__in_order_traverse(bt, node->childPointers[i + 1], func);
	}
	RELEASEBNODE(&node);
}

/*中序遍历-递归实现,*/
static void traverse(BTree* bt, BForEachFuncT func)
{
	__in_order_traverse(bt, bt->head.rootPointer, func);
}

static void insert(BTree* bt, const void* pKey, const void* pValue)
{

}

static void erase(BTree* bt, const void* pKey)
{

}

static const void* at(BTree* bt, const void* pKey)
{

}

static void change(BTree* bt, const void* pKey, const void* pValue)
{
	insert(bt, pKey, pValue);
}

inline const BTreeOp* GetBTreeOpStruct()
{
	static const BTreeOp OpList = {
		.create = create,
		.destroy = destroy,
		.level_order_traverse = level_order_traverse,
		.traverse = traverse,
		.insert = insert,
		.erase = erase,
		.at = at,
		.change = change,
	};
	return &OpList;
}