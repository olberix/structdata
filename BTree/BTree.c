#include "BTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "../DlQueue/DlQueue.h"
#include "../SqStack/SqStack.h"

extern int fsync(int);
extern int fdatasync(int);
extern int getpagesize();
#define PAGE_RATE 1 //实际页大小之内存页大小倍数
#define KEYSIZE (bt->head.keySize)
#define VALSIZE (bt->head.valSize)
#define PAGESIZE (bt->head.pageSize)
#define ROOTPOINTER (bt->head.rootPointer)

static inline BNode* NEWBNODE(BTree* bt)
{
	POINTCREATE_INIT(BNode*, node, BNode, sizeof(BNode));
	POINTCREATE_INIT(EMPTYDEF, node->childPointers, off_t, sizeof(off_t) * (bt->maxNC + 1));//childPointers清0,判断是否叶子结点
	POINTCREATE(EMPTYDEF, node->pKey, void, KEYSIZE * bt->maxNC);
	POINTCREATE(EMPTYDEF, node->pValue, void, VALSIZE * bt->maxNC);
	return node;
}

static void WRITEHEADER(BTree* bt)
{
	POINTCREATE_INIT(char*, tmpStr, char, PAGESIZE);
	memcpy(tmpStr, &(bt->head), sizeof(HeaderNode));
	lseek(bt->fd, 0, SEEK_SET);
	CONDCHECK(write(bt->fd, tmpStr, PAGESIZE) == PAGESIZE, STATUS_WRERROR, __FILE__, __LINE__);
	FREE(tmpStr);
}

/*存储顺序:size-pKey-pValue-childPointers,存储的size必然大于等于1*/
static inline void READNODE(BTree* bt, off_t pointer, BNode* node)
{
	lseek(bt->fd, pointer, SEEK_SET);
	node->selfPointer = pointer;
	ssize_t _size = sizeof(node->size);
	CONDCHECK(read(bt->fd, &(node->size), _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
	_size = node->size * KEYSIZE;
	CONDCHECK(read(bt->fd, node->pKey, _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
	_size = node->size * VALSIZE;
	CONDCHECK(read(bt->fd, node->pValue, _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
	_size = (node->size + 1) * sizeof(off_t);//文件读取node必然有值,页剩余空间用0填充,故这里childPointers不用清0
	CONDCHECK(read(bt->fd, node->childPointers, _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
}

static inline bool ISLEAF(BNode* node)
{
	return node->childPointers[0] == 0;
}

static inline void RELEASEBNODE(BNode** nnode)
{
	FREE((*nnode)->pValue);
	FREE((*nnode)->pKey);
	FREE((*nnode)->childPointers);
	FREE(*nnode);
}

/*
*2021/11/29 11:18:40 终于搞定这困扰足足一周的bug了
*bug:在设定val的size使每个结点只存储[1,3]个关键字,然后在对75w条数据的文件操作时,删除后再插入会出现数据丢失的情况
*这个数据丢失是偶现的,而且每次丢失的数据也不相同,因为这个随机性其实可以排除B树的删除和插入逻辑问题,因为这个B树的删除和插入实现
 不具有随机性,但我还是花了3.4天的时间调试这部分代码
*后来才想到会不会是fallocate和SEEK_HOLE的配合问题(其实一开始也想到是不是他们的问题,但很快意识中排除了,因为开始写B树前已经对他们进行过
 不少测试,但都是小文件操作,这也直接导致了花费足足一周时间的悲剧)
*导致原因:插入的时候,如果写入位置为新分配的位置A(空洞或者文件尾),在下一次的lseek(bt->fd, 0, SEEK_HOLE)的时候,偏移量依然可能会为A,
 这应该是内核文件缓存与磁盘可能不一致导致的?SEEK_HOLE以磁盘块为偏移,所以lseek(bt->fd, 0, SEEK_HOLE)的实现是读取inode中的block数组?这可以解释发生bug的随机性
*复现方式:注释掉WRITENODE函数最后两行,最后两行功能就是在新位置写入文件的时候同步一次内核与磁盘数据并等待返回,fdatasync会导致B树时间的急剧增加
 (在新分配位置时,用红黑树记录已分配过的位置,这样就知道下一次lseek(bt->fd, 0, SEEK_HOLE)是否分配到了已经分配过的位置)
*实现空洞写入数据更高效的方法应该是另外记录这个空洞位置,这样就不用每次同步磁盘了,下次B+树的实现打算采用这种方式
*/
static inline void WRITENODE(BTree* bt, BNode* node)
{
	if (node->size == 0)//等于0不再写,因为必然被fallocate
		return;
	bool sync = false;
	if (node->selfPointer)
		lseek(bt->fd, node->selfPointer, SEEK_SET);
	else{
		node->selfPointer = lseek(bt->fd, 0, SEEK_HOLE);
		sync = true;
	}
	CONDCHECK(node->selfPointer >= PAGESIZE && node->selfPointer % PAGESIZE == 0, STATUS_OFFSETERROR, __FILE__, __LINE__);
	POINTCREATE_INIT(char*, tmpStr, char, PAGESIZE);
	memcpy(tmpStr, &(node->size), sizeof(node->size));
	ssize_t _size = sizeof(node->size);
	memcpy(tmpStr + _size, node->pKey, KEYSIZE * node->size);
	_size += KEYSIZE * node->size;
	memcpy(tmpStr + _size, node->pValue, VALSIZE * node->size);
	if (!ISLEAF(node)){
		_size += VALSIZE * node->size;
		memcpy(tmpStr + _size, node->childPointers, (node->size + 1) * sizeof(off_t));
	}
	CONDCHECK(write(bt->fd, tmpStr, PAGESIZE) == PAGESIZE, STATUS_WRERROR, __FILE__, __LINE__);
	FREE(tmpStr);
	if (sync)
		fdatasync(bt->fd);
}

static inline void FILERELEASE(BTree* bt, BNode* node)
{
	CONDCHECK(fallocate(bt->fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, node->selfPointer, PAGESIZE) >= 0, STATUS_FALLOCATEERROR, __FILE__, __LINE__);
}

static BTree* create(size_t keySize, size_t valSize, BKeyCompareFuncT equalFunc, BKeyCompareFuncT lessFunc, const char* fileName)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR, __FILE__, __LINE__);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	POINTCREATE_INIT(BTree*, bt, BTree, sizeof(BTree));
	int flag = access(fileName, F_OK);
	bt->fd = open(fileName, O_RDWR | O_CREAT, 777);
	CONDCHECK(bt->fd > 0, STATUS_FDERROR, __FILE__, __LINE__);
	if (flag == 0){//文件存在,代表已经创建
		CONDCHECK(read(bt->fd, &(bt->head), sizeof(HeaderNode)) == sizeof(HeaderNode), STATUS_RDERROR, __FILE__, __LINE__);
		CONDCHECK(KEYSIZE == keySize && VALSIZE == valSize && PAGESIZE == getpagesize() * PAGE_RATE, STATUS_SIZEERROR, __FILE__, __LINE__);
	}
	else{
		KEYSIZE = keySize;
		VALSIZE = valSize;
		PAGESIZE = getpagesize() * PAGE_RATE;
		WRITEHEADER(bt);
	}
	POINTCREATE(EMPTYDEF, bt->tmpRet, void, valSize);
	bt->equalFunc = equalFunc;
	bt->lessFunc = lessFunc;
	bt->maxNC = (PAGESIZE - sizeof(off_t) - sizeof(size_t)) / (keySize + valSize + sizeof(off_t));
	__typeof__(((BTree*)NULL)->maxNC) t = (bt->maxNC + 1) / 2;//最小度数t>=2
	CONDCHECK(t >= 2, STATUS_DEERROR, __FILE__, __LINE__);//t要往下取整(0.5),若t往上取整,结点合成:t-1+t-2+1=2t-2,此时2(t+0.5)-2=2t-1刚好为最大结点数,
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
			for (ssize_t i = 0; i < node->size + 1; i++)
				DlQueue().push(queue, node->childPointers + i);
		for (ssize_t i = 0; i < node->size; i++)
			func(node->pKey + KEYSIZE * i, node->pValue + VALSIZE * i);
#ifdef DEBUG
		puts("-----");
#endif
	}
	RELEASEBNODE(&node);
	DlQueue().destroy(&queue);
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

/*中序遍历-递归实现*/
static inline void traverse(BTree* bt, BForEachFuncT func)
{
	__in_order_traverse(bt, ROOTPOINTER, func);
}

//二分法查找pKey位于对比结点的位置
static inline bool FINDKEYEXPECTLOC(BTree* bt, BNode* node, const void* pKey, ssize_t* loc)
{
	ssize_t left = 0, right = node->size;
	ssize_t last_mid = -100, mid;
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

/*结点某位置数据往后移动一格*/
static inline void MOVEBACKONESTEP(BTree* bt, BNode* node, ssize_t loc, ssize_t child_loc)
{
	if (loc < node->size){
		memmove(node->pKey + KEYSIZE * (loc + 1), node->pKey + KEYSIZE * loc, KEYSIZE * (node->size - loc));
		memmove(node->pValue + VALSIZE * (loc + 1), node->pValue + VALSIZE * loc, VALSIZE * (node->size - loc));
	}
	if (!ISLEAF(node) && child_loc < node->size + 1)
		memmove(node->childPointers + child_loc + 1, node->childPointers + child_loc, sizeof(off_t) * (node->size + 1 - child_loc));
}

/*结点某位置数据往前移动一格*/
static inline void MOVEFORWARDONESTEP(BTree* bt, BNode* node, ssize_t loc, ssize_t child_loc)
{
	if (loc > 0 && loc < node->size){
		memmove(node->pKey + KEYSIZE * (loc - 1), node->pKey + KEYSIZE * loc, KEYSIZE * (node->size - loc));
		memmove(node->pValue + VALSIZE * (loc - 1), node->pValue + VALSIZE * loc, VALSIZE * (node->size - loc));
	}
	if (!ISLEAF(node) && child_loc > 0 && child_loc < node->size + 1)
		memmove(node->childPointers + child_loc - 1, node->childPointers + child_loc, sizeof(off_t) * (node->size + 1 - child_loc));
}

static inline BNode* SPLITNODE(BTree* bt, BNode* node, BNode** pparent, ssize_t* rrp)
{
	BNode* spl = NULL;
	if (bt->maxNC <= node->size){
		spl = NEWBNODE(bt);
		const ssize_t raise_I = node->size / 2;
		const ssize_t spl_I = raise_I + 1;
		//移动node数据到分裂结点--begin--
		spl->size = node->size - 1 - raise_I;
		memcpy(spl->pKey, node->pKey + spl_I * KEYSIZE, spl->size * KEYSIZE);
		memcpy(spl->pValue, node->pValue + spl_I * VALSIZE, spl->size * VALSIZE);
		if (!ISLEAF(node))
			memcpy(spl->childPointers, node->childPointers + spl_I, (spl->size + 1) * sizeof(off_t));
		//--end--
		WRITENODE(bt, spl);
		if (*pparent){//非根节点
			FINDKEYEXPECTLOC(bt, *pparent, node->pKey, rrp);
			MOVEBACKONESTEP(bt, *pparent, *rrp, *rrp + 1);//移动父结点自身数据
			//raise_I坐标数据上移父结点
			memcpy((*pparent)->pKey + KEYSIZE * (*rrp), node->pKey + KEYSIZE * raise_I, KEYSIZE);
			memcpy((*pparent)->pValue + VALSIZE * (*rrp), node->pValue + VALSIZE * raise_I, VALSIZE);
			memcpy((*pparent)->childPointers + *rrp + 1, &(spl->selfPointer), sizeof(off_t));
			(*pparent)->size++;
			WRITENODE(bt, *pparent);
		}
		else{//根节点
			BNode* newRoot = NEWBNODE(bt);
			memcpy(newRoot->pKey, node->pKey + KEYSIZE * raise_I, KEYSIZE);
			memcpy(newRoot->pValue, node->pValue + VALSIZE * raise_I, VALSIZE);
			newRoot->childPointers[0] = node->selfPointer;
			newRoot->childPointers[1] = spl->selfPointer;
			newRoot->size = 1;
			WRITENODE(bt, newRoot);
			ROOTPOINTER = newRoot->selfPointer;
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
		ROOTPOINTER = node->selfPointer;
		WRITEHEADER(bt);
		RELEASEBNODE(&node);
		return;
	}
	off_t pointer = ROOTPOINTER;
	BNode* node = NEWBNODE(bt);
	BNode* parent = NULL;
	do{
		READNODE(bt, pointer, node);
		ssize_t rrp;//如果结点分割,rrp便赋值上升key在parent结点中的索引
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
		ssize_t loc;
		if (!FINDKEYEXPECTLOC(bt, node, pKey, &loc)){
			memcpy(node->pValue + loc * VALSIZE, pValue, VALSIZE);
			break;
		}
		if (ISLEAF(node)){
			MOVEBACKONESTEP(bt, node, loc, 0);
			memcpy(node->pKey + loc * KEYSIZE, pKey, KEYSIZE);
			memcpy(node->pValue + loc * VALSIZE, pValue, VALSIZE);
			node->size++;
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

static inline bool ERASE_FINDREPLACE(BTree* bt, const void* pKey, SqStack* stack_node, SqStack* stack_loc)
{
	BNode* node;//node为最终删除关键字所在的叶子结点,stack_node&stack_loc记录了路径信息并且包含了node
	ssize_t loc;
	off_t pointer = ROOTPOINTER;
	while(true){
		node = NEWBNODE(bt);
		READNODE(bt, pointer, node);
		bool ret = FINDKEYEXPECTLOC(bt, node, pKey, &loc);
		SqStack().push(stack_node, &node);
		if (ret){
			if (ISLEAF(node))
				return false;
			pointer = node->childPointers[loc];
			SqStack().push(stack_loc, &loc);
			continue;
		}
		if (!ISLEAF(node)){
			BNode* rplc;
			pointer = node->childPointers[loc];//找前驱替代,这样替代的叶子结点可以直接改size,而不用移动数据
			SqStack().push(stack_loc, &loc);
			do{
				rplc = NEWBNODE(bt);
				READNODE(bt, pointer, rplc);
				SqStack().push(stack_node, &rplc);
				if (ISLEAF(rplc)){
					memcpy(node->pKey + loc * KEYSIZE, rplc->pKey + (rplc->size - 1) * KEYSIZE, KEYSIZE);
					memcpy(node->pValue + loc * VALSIZE, rplc->pValue + (rplc->size - 1) * VALSIZE, VALSIZE);
					WRITENODE(bt, node);
					node = rplc;
					loc = rplc->size - 1;
					break;
				}
				pointer = rplc->childPointers[rplc->size];
				SqStack().push(stack_loc, &(rplc->size));
			}while(true);
		}
		if (loc != node->size - 1){
			ssize_t diff = node->size - 1 - loc;
			memmove(node->pKey + loc * KEYSIZE, node->pKey + (loc + 1) * KEYSIZE, diff * KEYSIZE);
			memmove(node->pValue + loc * VALSIZE, node->pValue + (loc + 1) * VALSIZE, diff * VALSIZE);
		}
		node->size--;
		WRITENODE(bt, node);
		return true;
	}
}

static inline void ERASE_BALANCE(BTree* bt, SqStack* stack_node, SqStack* stack_loc)
{
	BNode* sbl = NEWBNODE(bt);
	BNode* node;
	do{
		node = TOCONSTANT(BNode*, SqStack().pop(stack_node));
		if (node->size >= bt->minNC)//停止回溯
			break;
		if (SqStack().empty(stack_loc)){//到达了根节点
			if (node->size == 0){
				ROOTPOINTER = node->childPointers[0];
				WRITEHEADER(bt);
				FILERELEASE(bt, node);
			}
			break;
		}
		ssize_t loc = TOCONSTANT(ssize_t, SqStack().pop(stack_loc));
		BNode* parent = TOCONSTANT(BNode*, SqStack().get_top(stack_node));
		bool ret = false;
		ssize_t mploc;
		BNode* mlt = NULL, *mrt = NULL;
		//这里是一个艰难的抉择,究竟是判断一个兄弟不够就合并还是判断完两个兄弟不够再选择一个合并
		//前者做法可以让树的结点变少但是可能会大概率增加回溯路程,后者反之(但会稳定多出一次系统调用)
		//无法预测对B树删除之后的操作,那就选择对当前删除操作较为稳定的后者做法
		if (loc > 0){//优先往左借,往哪边借都需要移动数据,但是往左借移动自身t-2个数据,往右借移动兄弟大于等于t-1个数据
			mploc = loc - 1;
			READNODE(bt, parent->childPointers[mploc], sbl);
			if(sbl->size > bt->minNC){
				MOVEBACKONESTEP(bt, node, 0, 0);
				memcpy(node->pKey, parent->pKey + KEYSIZE * mploc, KEYSIZE);
				memcpy(node->pValue, parent->pValue + VALSIZE * mploc, VALSIZE);
				memcpy(parent->pKey + KEYSIZE * mploc, sbl->pKey + KEYSIZE * (sbl->size - 1), KEYSIZE);
				memcpy(parent->pValue + VALSIZE * mploc, sbl->pValue + VALSIZE * (sbl->size - 1), VALSIZE);
				if (!ISLEAF(node))
					memcpy(node->childPointers, sbl->childPointers + sbl->size, sizeof(off_t));
				sbl->size--;
				node->size++;
				WRITENODE(bt, parent);
				WRITENODE(bt, node);
				WRITENODE(bt, sbl);
				ret = true;
			}
			else{
				mlt = sbl;
				mrt = node;
			}
		}
		if (!ret && loc != parent->size){
			mploc = loc;
			READNODE(bt, parent->childPointers[mploc + 1], sbl);
			if(sbl->size > bt->minNC){
				memcpy(node->pKey + KEYSIZE * node->size, parent->pKey + KEYSIZE * mploc, KEYSIZE);
				memcpy(node->pValue + VALSIZE * node->size, parent->pValue + VALSIZE * mploc, VALSIZE);
				memcpy(parent->pKey + KEYSIZE * mploc, sbl->pKey, KEYSIZE);
				memcpy(parent->pValue + VALSIZE * mploc, sbl->pValue, VALSIZE);
				if (!ISLEAF(node))
					memcpy(node->childPointers + node->size + 1, sbl->childPointers, sizeof(off_t));
				node->size++;
				MOVEFORWARDONESTEP(bt, sbl, 1, 1);
				sbl->size--;
				WRITENODE(bt, parent);
				WRITENODE(bt, node);
				WRITENODE(bt, sbl);
				ret = true;
			}
			else{
				mlt = node;
				mrt = sbl;
			}
		}
		if (!ret){//合并结点
			memcpy(mlt->pKey + KEYSIZE * mlt->size, parent->pKey + KEYSIZE * mploc, KEYSIZE);
			memcpy(mlt->pValue + VALSIZE * mlt->size, parent->pValue + VALSIZE * mploc, VALSIZE);
			mlt->size++;
			memcpy(mlt->pKey + KEYSIZE * mlt->size, mrt->pKey, KEYSIZE * mrt->size);
			memcpy(mlt->pValue + VALSIZE * mlt->size, mrt->pValue, VALSIZE * mrt->size);
			if (!ISLEAF(mlt))
				memcpy(mlt->childPointers + mlt->size, mrt->childPointers, sizeof(off_t) * (mrt->size + 1));
			mlt->size += mrt->size;
			MOVEFORWARDONESTEP(bt, parent, mploc + 1, mploc + 2);
			parent->size--;
			WRITENODE(bt, parent);
			WRITENODE(bt, mlt);
			FILERELEASE(bt, mrt);
		}
		RELEASEBNODE(&node);
	}while(!SqStack().empty(stack_node));
	RELEASEBNODE(&node);
	RELEASEBNODE(&sbl);
}

static void erase(BTree* bt, const void* pKey)
{
	if (!ROOTPOINTER)
		return;
	SqStack* stack_node = SqStack().create(sizeof(BNode*), NULL);
	SqStack* stack_loc = SqStack().create(sizeof(ssize_t), NULL);
	bool ret = ERASE_FINDREPLACE(bt, pKey, stack_node, stack_loc);
	if (ret)
		ERASE_BALANCE(bt, stack_node, stack_loc);
	//释放空间返回
	while(!SqStack().empty(stack_node)){
		BNode* vv = TOCONSTANT(BNode*, SqStack().pop(stack_node));
		RELEASEBNODE(&vv);
	}
	SqStack().destroy(&stack_node);
	SqStack().destroy(&stack_loc);
}

static const void* at(BTree* bt, const void* pKey)
{
	if (!ROOTPOINTER)
		return NULL;
	off_t pointer = ROOTPOINTER;
	BNode* node = NEWBNODE(bt);
	do{
		READNODE(bt, pointer, node);
		ssize_t loc;
		if (FINDKEYEXPECTLOC(bt, node, pKey, &loc)){
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