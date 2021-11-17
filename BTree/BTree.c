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
#define ROOTPOINTER (bt->head.rootPointer)

static void WRITEHEADER(BTree* bt)
{
	POINTCREATE_INIT(char*, tmp, char, PAGESIZE);
	memcpy(tmp, &(bt->head), sizeof(HeaderNode));
	lseek(bt->fd, 0, SEEK_SET);
	CONDCHECK(write(bt->fd, tmp, PAGESIZE) == PAGESIZE, STATUS_WRERROR);
}

static BTree* create(size_t keySize, size_t valSize, BKeyCompareFuncT equalFunc, BKeyCompareFuncT lessFunc, const char* fileName)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC);
	POINTCREATE_INIT(BTree*, bt, BTree, sizeof(BTree));
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
		WRITEHEADER(bt);
	}
	POINTCREATE(EMPTYDEF, bt->tmpRet, void, valSize);
	bt->equalFunc = equalFunc;
	bt->lessFunc = lessFunc;
	bt->maxNC = (PAGESIZE - sizeof(off_t) - sizeof(size_t)) / (keySize + valSize + sizeof(off_t));
	__typeof__(bt->maxNC) t = (bt->maxNC + 1) / 2;//最小度数t>=2
	CONDCHECK(t >= 2, STATUS_DEERROR);//t要往下取整(0.5),若t往上取整,结点合成:t-1+t-2+1=2t-2,此时2(t+0.5)-2=2t-1刚好为最大结点数,
	bt->minNC = t - 1;//但结点分解时,即分解成两个t-1,这时候没有多余结点合并到父结点,不符合逻辑
	return bt;//同理t往下取整,合并和分解都符合B树性质,故t往下取整
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
	__typeof__(node->size) _size = sizeof(node->size);
	CONDCHECK(read(bt->fd, &(node->size), _size) == _size, STATUS_RDERROR);
	_size = node->size * KEYSIZE;
	CONDCHECK(read(bt->fd, node->pKey, _size) == _size, STATUS_RDERROR);
	_size = node->size * VALSIZE;
	CONDCHECK(read(bt->fd, node->pValue, _size) == _size, STATUS_RDERROR);
	_size = (node->size + 1) * sizeof(off_t);//文件读取node必然有值,故这里childPointers不用清0
	CONDCHECK(read(bt->fd, node->childPointers, _size) == _size, STATUS_RDERROR);
}

static inline off_t WRITENODE(BTree* bt, BNode* node)
{
	off_t offset = lseek(bt->fd, 0, SEEK_HOLE);
	CONDCHECK(offset > 0 && offset % PAGESIZE == 0, STATUS_RDERROR);
	POINTCREATE_INIT(char*, tmp, char, PAGESIZE);
	memcpy(tmp, &(node->size), sizeof(node->size));
	__typeof__(node->size) _size = sizeof(node->size);
	memcpy(tmp + _size, node->pKey, KEYSIZE * node->size);
	_size += KEYSIZE * node->size;
	memcpy(tmp + _size, node->pValue, VALSIZE * node->size);
	_size += VALSIZE * node->size;
	memcpy(tmp + _size, node->childPointers, (node->size + 1) * sizeof(off_t));
	CONDCHECK(write(bt->fd, tmp, PAGESIZE) == PAGESIZE, STATUS_RDERROR);
	return offset;
}

static inline bool ISLEAF(BNode* node)
{
	return node->childPointers[0] == 0;
}

static inline BNode* NEWBNODE(BTree* bt)
{
	POINTCREATE(BNode*, node, BNode, sizeof(BNode));
	POINTCREATE_INIT(EMPTYDEF, node->childPointers, off_t, sizeof(off_t) * (bt->maxNC + 1));//childPointers清0,判断是否叶子结点
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
	if (!ROOTPOINTER)
		return;
	DlQueue* queue = DlQueue().create(sizeof(off_t));
	DlQueue().push(queue, &ROOTPOINTER);
	BNode* node = NEWBNODE(bt);
	while(!DlQueue().empty(queue)){
		off_t pointer = TOCONSTANT(off_t, DlQueue().pop(queue));
		READNODE(bt, pointer, node);
		if (!ISLEAF(node))
			for (__typeof__(node->size) i = 0; i < node->size + 1; i++)
				DlQueue().push(queue, node->childPointers + i);
		for (__typeof__(node->size) i = 0; i < node->size; i++)
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
		for(__typeof__(node->size) i = 0; i < node->size; i++)
			func(node->pKey + i * KEYSIZE, node->pValue + i * VALSIZE);
		RELEASEBNODE(&node);
		return;
	}
	__in_order_traverse(bt, node->childPointers[0], func);
	for (__typeof__(node->size) i = 0; i < node->size; i++){
		func(node->pKey + i * KEYSIZE, node->pValue + i * VALSIZE);
		__in_order_traverse(bt, node->childPointers[i + 1], func);
	}
	RELEASEBNODE(&node);
}

/*中序遍历-递归实现*/
static inline void traverse(BTree* bt, BForEachFuncT func)
{
	__in_order_traverse(bt, ROOTPOINTER, func);
}

static inline BNode* SPLITNODE(BTree* bt, BNode* node)
{
	BNode* spl = NULL;
	if (bt->maxNC <= node->size){
		spl = NEWBNODE(bt);
		__typeof__(node->size) pt = node->size / 2;

	}
	return spl;
}

static void insert(BTree* bt, const void* pKey, const void* pValue)
{
	if (!ROOTPOINTER){
		BNode* node = NEWBNODE(bt);
		node->size = 1;
		memcpy(node->pKey, pKey, KEYSIZE);
		memcpy(node->pValue, pValue, VALSIZE);
		off_t offset = WRITENODE(bt, node);
		ROOTPOINTER = offset;
		WRITEHEADER(bt);
		RELEASEBNODE(&node);
		return;
	}
	off_t pointer = ROOTPOINTER;
	BNode* node = NEWBNODE(bt);
	do{
		READNODE(bt, pointer, node);
		BNode* spl = SPLITNODE(bt, node);
		if (spl){

		}
		if (ISLEAF(node)){

		}
		else{

		}
	}while(true);
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