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

/*存储顺序:size-pKey-pValue-childPointers*/
static inline void READNODE(BTree* bt, off_t pointer, BNode* node)
{
	lseek(bt->fd, pointer, SEEK_SET);
	node->selfPoint = pointer;
	BNodeST _size = sizeof(node->size);
	CONDCHECK(read(bt->fd, &(node->size), _size) == _size, STATUS_RDERROR);
	_size = node->size * KEYSIZE;
	CONDCHECK(read(bt->fd, node->pKey, _size) == _size, STATUS_RDERROR);
	_size = node->size * VALSIZE;
	CONDCHECK(read(bt->fd, node->pValue, _size) == _size, STATUS_RDERROR);
	_size = (node->size + 1) * sizeof(off_t);//文件读取node必然有值,故这里childPointers不用清0
	CONDCHECK(read(bt->fd, node->childPointers, _size) == _size, STATUS_RDERROR);
}

static inline bool ISLEAF(BNode* node)
{
	return node->childPointers[0] == 0;
}

static inline void WRITENODE(BTree* bt, BNode* node)
{
	off_t offset = node->selfPoint;
	if (!offset){
		offset = lseek(bt->fd, 0, SEEK_HOLE);
		node->selfPoint = offset;
	}
	else
		lseek(bt->fd, offset, SEEK_SET);
	CONDCHECK(offset > 0 && offset % PAGESIZE == 0, STATUS_OFFSETERROR);
	POINTCREATE_INIT(char*, tmp, char, PAGESIZE);
	memcpy(tmp, &(node->size), sizeof(node->size));
	BNodeST _size = sizeof(node->size);
	memcpy(tmp + _size, node->pKey, KEYSIZE * node->size);
	_size += KEYSIZE * node->size;
	memcpy(tmp + _size, node->pValue, VALSIZE * node->size);
	if (!ISLEAF(node)){
		_size += VALSIZE * node->size;
		memcpy(tmp + _size, node->childPointers, (node->size + 1) * sizeof(off_t));
	}
	CONDCHECK(write(bt->fd, tmp, PAGESIZE) == PAGESIZE, STATUS_WRERROR);
}

static inline BNode* NEWBNODE(BTree* bt)
{
	POINTCREATE_INIT(BNode*, node, BNode, sizeof(BNode));
	POINTCREATE_INIT(EMPTYDEF, node->childPointers, off_t, sizeof(off_t) * (bt->maxNC + 1));//childPointers清0,判断是否叶子结点
	POINTCREATE(EMPTYDEF, node->pKey, void, KEYSIZE * bt->maxNC);
	POINTCREATE(EMPTYDEF, node->pValue, void, VALSIZE * bt->maxNC);
	return node;
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
		// CONDCHECK(KEYSIZE == keySize && VALSIZE == valSize && PAGESIZE == 160, STATUS_SIZEERROR);
	}
	else{
		KEYSIZE = keySize;
		VALSIZE = valSize;
		PAGESIZE = getpagesize();
		// PAGESIZE = 160;
		WRITEHEADER(bt);
	}
	POINTCREATE(EMPTYDEF, bt->tmpRet, void, valSize);
	bt->equalFunc = equalFunc;
	bt->lessFunc = lessFunc;
	bt->maxNC = (PAGESIZE - sizeof(off_t) - sizeof(size_t)) / (keySize + valSize + sizeof(off_t));
	__typeof__(((BTree*)NULL)->maxNC) t = (bt->maxNC + 1) / 2;//最小度数t>=2
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
			for (BNodeST i = 0; i < node->size + 1; i++)
				DlQueue().push(queue, node->childPointers + i);
		for (BNodeST i = 0; i < node->size; i++)
			func(node->pKey + KEYSIZE * i, node->pValue + VALSIZE * i);
#ifdef DEBUG
		puts("-----");
#endif
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
		for(BNodeST i = 0; i < node->size; i++)
			func(node->pKey + i * KEYSIZE, node->pValue + i * VALSIZE);
		RELEASEBNODE(&node);
		return;
	}
	__in_order_traverse(bt, node->childPointers[0], func);
	for (BNodeST i = 0; i < node->size; i++){
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

/*
二分法查找pKey位于对比结点的位置
todo这个破函数死循环了n次,目前看来已经正常了,但总觉得有更好的实现
*/
static inline bool FINDCHILDLOCATION(BTree* bt, BNode* node, const void* pKey, BNodeST* loc)
{
	BNodeST left = 0, right = node->size;
	BNodeST last_mid = -100, mid;
	do{
		mid = (right + left) / 2;
		if (bt->equalFunc(node->pKey + mid * KEYSIZE, pKey)){
			*loc = mid;
			return false;
		}
		if (bt->lessFunc(node->pKey + mid * KEYSIZE, pKey)){
			if (mid == 0 || last_mid + 1 == mid){
				*loc = mid;
				return true;
			}
			right = mid;
		}
		else{
			if (last_mid == mid){
				*loc = right;
				return true;
			}
			if (last_mid - 1 == mid){
				*loc = last_mid;
				return true;
			}
			left = mid;
		}
		last_mid = mid;
	}while(true);
}

static inline void MOVE2SELF(BTree* bt, BNode* node, BNodeST loc)
{
	BNodeST diff = node->size - loc;
	memmove(node->pKey + KEYSIZE * (loc + 1), node->pKey + KEYSIZE * loc, KEYSIZE * diff);
	memmove(node->pValue + VALSIZE * (loc + 1), node->pValue + VALSIZE * loc, VALSIZE * diff);
	if (!ISLEAF(node))
		memmove(node->childPointers + loc + 2, node->childPointers + loc + 1, diff);
}

static inline BNode* SPLITNODE(BTree* bt, BNode* node, BNode** pparent, BNodeST* rrp)
{
	BNode* spl = NULL;
	if (bt->maxNC <= node->size){
		spl = NEWBNODE(bt);
		const BNodeST raise_I = node->size / 2;
		const BNodeST spl_I = raise_I + 1;
		//移动node数据到分裂结点--begin--
		spl->size = node->size - 1 - raise_I;
		memcpy(spl->pKey, node->pKey + spl_I * KEYSIZE, spl->size * KEYSIZE);
		memcpy(spl->pValue, node->pValue + spl_I * VALSIZE, spl->size * VALSIZE);
		if (!ISLEAF(node))
			memcpy(spl->childPointers, node->childPointers + spl_I, (spl->size + 1) * sizeof(off_t));
		//--end--
		WRITENODE(bt, spl);
		if (*pparent){//非根节点
			FINDCHILDLOCATION(bt, *pparent, node->pKey, rrp);
			if (*rrp < (*pparent)->size)//移动父结点自身数据
				MOVE2SELF(bt, *pparent, *rrp);
			//raise_I坐标数据上移父结点
			memcpy((*pparent)->pKey + KEYSIZE * (*rrp), node->pKey + KEYSIZE * raise_I, KEYSIZE);
			memcpy((*pparent)->pValue + VALSIZE * (*rrp), node->pValue + VALSIZE * raise_I, VALSIZE);
			memcpy((*pparent)->childPointers + (*rrp) + 1, &(spl->selfPoint), sizeof(off_t));
			(*pparent)->size += 1;
			WRITENODE(bt, *pparent);
		}
		else{//根节点
			BNode* newRoot = NEWBNODE(bt);
			memcpy(newRoot->pKey, node->pKey + KEYSIZE * raise_I, KEYSIZE);
			memcpy(newRoot->pValue, node->pValue + VALSIZE * raise_I, VALSIZE);
			newRoot->childPointers[0] = node->selfPoint;
			newRoot->childPointers[1] = spl->selfPoint;
			newRoot->size = 1;
			WRITENODE(bt, newRoot);
			ROOTPOINTER = newRoot->selfPoint;
			WRITEHEADER(bt);
			*pparent = newRoot;
			*rrp = 0;
		}
		node->size -= (spl->size + 1);
		WRITENODE(bt, node);
	}
	return spl;
}

static void insert(BTree* bt, const void* pKey, const void* pValue)
{
	if (!ROOTPOINTER){
		BNode* node = NEWBNODE(bt);
		memcpy(node->pKey, pKey, KEYSIZE);
		memcpy(node->pValue, pValue, VALSIZE);
		node->size = 1;
		WRITENODE(bt, node);
		ROOTPOINTER = node->selfPoint;
		WRITEHEADER(bt);
		RELEASEBNODE(&node);
		return;
	}
	off_t pointer = ROOTPOINTER;
	BNode* node = NEWBNODE(bt);
	BNode* parent = NULL;
	do{
		READNODE(bt, pointer, node);
		BNodeST rrp;//如果结点分割,rrp便赋值上升key在parent结点中的索引
		BNode* spl = SPLITNODE(bt, node, &parent, &rrp);
		if (spl){//如果spl有值返回,parent必不为NULL
			if (bt->equalFunc(parent->pKey + rrp * KEYSIZE, pKey)){
				memcpy(parent->pValue + rrp * VALSIZE, pValue, VALSIZE);
				WRITENODE(bt, parent);
				RELEASEBNODE(&node);
				RELEASEBNODE(&parent);
				return;
			}
 			if (!bt->lessFunc(parent->pKey + rrp * KEYSIZE, pKey)){
 				RELEASEBNODE(&node);
 				node = spl;
 			}
 		}
		BNodeST loc;
		if (!FINDCHILDLOCATION(bt, node, pKey, &loc)){
			memcpy(node->pValue + loc * VALSIZE, pValue, VALSIZE);
			break;
		}
		if (ISLEAF(node)){
			if(loc < node->size)
				MOVE2SELF(bt, node, loc);
			memcpy(node->pKey + loc * KEYSIZE, pKey, KEYSIZE);
			memcpy(node->pValue + loc * VALSIZE, pValue, VALSIZE);
			node->size += 1;
			break;
		}
		pointer = node->childPointers[loc];
		BNode* tmp = parent;
		parent = node;
		node = tmp;
		if (!node)
			node = NEWBNODE(bt);
	}while(true);
	WRITENODE(bt, node);
	RELEASEBNODE(&node);
	if (parent)
		RELEASEBNODE(&parent);
}

static void erase(BTree* bt, const void* pKey)
{

}

static const void* at(BTree* bt, const void* pKey)
{
	if (!ROOTPOINTER)
		return NULL;
	off_t pointer = ROOTPOINTER;
	BNode* node = NEWBNODE(bt);
	do{
		READNODE(bt, pointer, node);
		BNodeST loc;
		if (FINDCHILDLOCATION(bt, node, pKey, &loc)){
			if (ISLEAF(node)){
				RELEASEBNODE(&node);
				return NULL;
			}
			pointer = node->childPointers[loc];
		}
		else{
			memcpy(bt->tmpRet, node->pValue + loc * VALSIZE, VALSIZE);
			RELEASEBNODE(&node);
			return bt->tmpRet;
		}
	}while(true);
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