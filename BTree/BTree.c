#include "BTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "../DlQueue/DlQueue.h"

extern int fsync(int);
extern int getpagesize();

static BTree* create(size_t keySize, size_t valSize, BKeyCompareFuncT equalFunc, BKeyCompareFuncT lessFunc, const char* fileName)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC);
	POINTCREATE(BTree*, bt, BTree, sizeof(BTree));
	memset(bt, 0, sizeof(BTree));
	int flag = access(fileName, F_OK);
	bt->fd = open(fileName, O_RDWR | O_CREAT, 777);
	CONDCHECK(bt->fd > 0, STATUS_FDERROR);
	HeaderNode* pH = &(bt->head);
	if (flag == 0){//文件存在,代表已经创建
		ssize_t num = read(bt->fd, pH, sizeof(HeaderNode));
		CONDCHECK(num == sizeof(HeaderNode), STATUS_RDERROR);
		CONDCHECK(pH->keySize == keySize && pH->valSize == valSize && pH->pageSize == getpagesize(), STATUS_SIZEERROR);
	}
	else{
		pH->keySize = keySize;
		pH->valSize = valSize;
		pH->pageSize = getpagesize();
		ssize_t num = write(bt->fd, pH, sizeof(HeaderNode));
		CONDCHECK(num == sizeof(HeaderNode), STATUS_WRERROR);
	}
	POINTCREATE(EMPTYDEF, bt->tmpRet, void, valSize);
	bt->equalFunc = equalFunc;
	bt->lessFunc = lessFunc;
	bt->maxNC = (pH->pageSize - sizeof(off_t) - sizeof(size_t)) / (keySize + valSize + sizeof(off_t));
	CONDCHECK((bt->maxNC + 1) / 2 >= 2, STATUS_DEERROR);
	bt->minNC = ((float)bt->maxNC + 1.5f) / 2.0f - 1;//结点合成:t-1+t-2+1=2t-2,遇到非整除情况t向上取整多出0.5,此时2(t+0.5)-2=2t-1刚好为最大结点数,达到最优布局
	return bt;//t往下取整也行,因为最小度数t>=2,所以2t-3>=t-1,同样满足B树性质
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
	CONDCHECK(read(bt->fd, &(node->size), sizeof(node->size)) == sizeof(node->size), STATUS_RDERROR);
	CONDCHECK(read(bt->fd, &(node->pKey), node->size) == node->size, STATUS_RDERROR);
	CONDCHECK(read(bt->fd, &(node->pValue), node->size) == node->size, STATUS_RDERROR);
	CONDCHECK(read(bt->fd, &(node->childPointers), node->size + 1) == node->size + 1, STATUS_RDERROR);
}

static void level_order_traverse(BTree* bt, BForEachFuncT func)
{
	off_t rootPointer = bt->head.rootPointer;
	if (!rootPointer)
		return;
	DlQueue* queue = DlQueue().create(sizeof(off_t));
	DlQueue().push(queue, &rootPointer);
	while(!DlQueue().empty(queue)){
		off_t pointer = TOCONSTANT(off_t, DlQueue().pop(queue));
		BNode node;
		READNODE(bt, pointer, &node);
		for (ssize_t i = 0; i < node.size + 1; i++)
			DlQueue().push(queue, node.childPointers + i);
		for (ssize_t i = 0; i < node.size; i++)
			func(node.pKey + bt->head.keySize * i, node.pValue + bt->head.valSize * i);
	}
}

/*中序遍历-递归实现,*/
static void traverse(BTree* bt, BForEachFuncT func)
{
	
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