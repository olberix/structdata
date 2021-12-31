#include "BPTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <math.h>
#include "../SqStack/SqStack.h"

//以一个字节整数为索引,记录这个整数从左到右第几个bit位为0
static const unsigned char bit_index_map[255] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,
4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,7,7,8
};
extern int fsync(int);
extern double ceil(double);
extern int getpagesize();
//元文件映射长度,这里映射1G,以结点最小关键数取极端值2,数据页最大数据行数取极端值1为例,1G映射能保存约57亿条数据(1G*8*2/3)
#define MMAP_SIZE 1024 * 1024 * 1024
//索引页大小之内存页大小倍数
#define INDEX_PAGE_RATE 2
//数据页大小之内存页大小倍数
#define DATA_PAGE_RATE 2
#define KEYSIZE (bt->meta.keySize)
#define VALSIZE (bt->meta.valSize)
#define META_PAGESIZE (bt->meta.metaPageSize)
#define INDEX_PAGESIZE (bt->meta.indexPageSize)
#define DATA_PAGESIZE (bt->meta.dataPageSize)
#define ROOTPOINTER (bt->meta.rootPointer)
#define FIRSTPOINTER (bt->meta.firstPointer)
#define METAFILESIZE (bt->meta.fileSize)
#define INDEXBITMAPEDGE (bt->meta.indexBitMapEdge)
#define KEYNODECOUNT_MAX (bt->meta.maxNC)
#define KEYNODECOUNT_MIN (bt->meta.minNC)
#define VALUECOUNT_MAX (bt->meta.maxDC)
#define DATAPAGEBITBYTES (bt->meta.dataPageBitBytes)

#define GETALLFILENAME(path)\
	strcat(bt->META_FILENAME, path);\
	strcat(bt->META_FILENAME, ".ccMeta");\
	strcat(bt->INDEX_FILENAME, path);\
	strcat(bt->INDEX_FILENAME, ".ccIndex");\
	strcat(bt->DATA_FILENAME, path);\
	strcat(bt->DATA_FILENAME, ".ccData");

//扩展元文件
static inline void FILE_EXTENDMETA(BPTree* bt)
{
	METAFILESIZE += META_PAGESIZE;
	ftruncate(bt->fdMeta, METAFILESIZE);
}

//是否叶子结点
static inline bool ISLEAF(BPNode* node)
{
	return node->isLeaf;
}

//申请新结点
static inline BPNode* NEWBPNODE(BPTree* bt)
{
	__keynode_size_t length = KEYNODECOUNT_MAX + 1;//插入时若结点已满先执行插入再执行分解
	POINTCREATE_INIT(BPNode*, node, BPNode, sizeof(BPNode));
	POINTCREATE(EMPTYDEF, node->childPointers, off_t, sizeof(off_t) * (length + 1));
	POINTCREATE(EMPTYDEF, node->pKey, void, KEYSIZE * length);
	node->selfPointer = -1;
	return node;
}

//释放结点空间
static inline void RELEASEBPNODE(BPNode** nnode)
{
	FREE((*nnode)->pKey);
	FREE((*nnode)->childPointers);
	FREE(*nnode);
}

//申请data文件空闲地址
static inline off_t __apply_valid_value_address(BPTree* bt)
{
	long long idxRoute = 0;
	unsigned char* tmp = bt->metaMap + META_PAGESIZE;
	while(true){
		if (((char*)tmp - bt->metaMap) >= METAFILESIZE)
			FILE_EXTENDMETA(bt);
		unsigned char* edgeByte = tmp + META_PAGESIZE;
		for(tmp += INDEXBITMAPEDGE; tmp < edgeByte; tmp++){
			if (tmp + 7 < edgeByte && *(unsigned long long*)tmp == 0xFFFFFFFFFFFFFFFF){
				tmp += 7;
				idxRoute += 64;
			}
			else{
				if (*tmp == 0xFF)
					idxRoute += 8;
				else{
					unsigned char n = bit_index_map[(unsigned char)(*tmp)];
					idxRoute += n;
					return (idxRoute - 1) * DATA_PAGESIZE;
				}
			}
		}
	}
}

//申请index文件空闲地址
static inline off_t __apply_valid_keynode_address(BPTree* bt)
{
	long long idxRoute = 0;
	unsigned char* tmp = bt->metaMap + META_PAGESIZE;
	while(true){
		if (((char*)tmp - bt->metaMap) >= METAFILESIZE)
			FILE_EXTENDMETA(bt);
		unsigned char* edgeByte = tmp + INDEXBITMAPEDGE;
		for(; tmp < edgeByte; tmp++){
			if (tmp + 7 < edgeByte && *(unsigned long long*)tmp == 0xFFFFFFFFFFFFFFFF){
				tmp += 7;
				idxRoute += 64;
			}
			else{
				if (*tmp == 0xFF)
					idxRoute += 8;
				else{
					unsigned char n = bit_index_map[(unsigned char)(*tmp)];
					idxRoute += n;
					*tmp |= 1 << (8 - n);
					return (idxRoute - 1) * INDEX_PAGESIZE;
				}
			}
		}
		tmp += (META_PAGESIZE - INDEXBITMAPEDGE);
	}
}

//data位图索引值置0或1
static inline void __release_value_address(BPTree* bt, off_t addr)
{
	long long idxRoute = addr / DATA_PAGESIZE + 1;
	long long idxByte = idxRoute / 8 + 1;
	unsigned char idxBit = idxRoute % 8;
	if (!idxBit){
		idxByte--;
		idxBit = 8;
	}
	int valueBytesPage = META_PAGESIZE - INDEXBITMAPEDGE;
	long long page = idxByte / valueBytesPage;
	idxByte = idxByte % valueBytesPage;
	if (!idxByte){
		page--;
		idxByte = valueBytesPage;
	}
	unsigned char* tmp = bt->metaMap + META_PAGESIZE + page * META_PAGESIZE + INDEXBITMAPEDGE + idxByte - 1;
	*tmp ^= 1 << (8 - idxBit);
}

//index位图索引值置0
static inline void __release_keynode_address(BPTree* bt, off_t addr)
{
	long long idxRoute = addr / INDEX_PAGESIZE + 1;
	long long idxByte = idxRoute / 8 + 1;
	unsigned char idxBit = idxRoute % 8;
	if (!idxBit){
		idxByte--;
		idxBit = 8;
	}
	long long page = idxByte / INDEXBITMAPEDGE;
	idxByte = idxByte % INDEXBITMAPEDGE;
	if (!idxByte){
		page--;
		idxByte = INDEXBITMAPEDGE;
	}
	unsigned char* tmp = bt->metaMap + META_PAGESIZE + page * META_PAGESIZE + idxByte - 1;
	*tmp ^= 1 << (8 - idxBit);
}

//写data文件
static inline off_t FILE_VALUEWRITE(BPTree* bt, const void* pValue)
{
	off_t pagePointer = __apply_valid_value_address(bt);
	POINTCREATE_INIT(char*, dataStr, char, DATA_PAGESIZE);
	if (pagePointer < lseek(bt->fdData, 0, SEEK_END)){
		lseek(bt->fdData, pagePointer, SEEK_SET);
		CONDCHECK(read(bt->fdData, dataStr, DATA_PAGESIZE) == (ssize_t)DATA_PAGESIZE, STATUS_RDERROR, __FILE__, __LINE__);
	}
	__value_size_t size;
	memcpy(&size, dataStr, sizeof(__value_size_t));
	unsigned char* edgeStr = dataStr + sizeof(__value_size_t) + DATAPAGEBITBYTES;
	long long idx = 0;
	for(unsigned char* tmp = dataStr + sizeof(__value_size_t); tmp < edgeStr; tmp++){
		if (tmp + 7 < edgeStr && *(unsigned long long*)tmp == 0xFFFFFFFFFFFFFFFF){
			tmp += 7;
			idx += 64;
		}
		else{
			if (*tmp == 0xFF)
				idx += 8;
			else{
				unsigned char n = bit_index_map[(unsigned char)(*tmp)];
				idx += n;
				*tmp |= 1 << (8 - n);
				break;
			}
		}
	}
	if(++size == VALUECOUNT_MAX)
		__release_value_address(bt, pagePointer);
	off_t logicalPointer = sizeof(__value_size_t) + DATAPAGEBITBYTES + (idx - 1) * VALSIZE;
	memcpy(dataStr + logicalPointer, pValue, VALSIZE);
	memcpy(dataStr, &size, sizeof(__value_size_t));
	lseek(bt->fdData, pagePointer, SEEK_SET);
	CONDCHECK(write(bt->fdData, dataStr, DATA_PAGESIZE) == (ssize_t)DATA_PAGESIZE, STATUS_WRERROR, __FILE__, __LINE__);
	FREE(dataStr);
	return pagePointer + logicalPointer;
}

//重写data文件
static inline void FILE_VALUECOVER(BPTree* bt, off_t valPointer, const void* pValue)
{
	lseek(bt->fdData, valPointer, SEEK_SET);
	CONDCHECK(write(bt->fdData, pValue, VALSIZE) == (ssize_t)VALSIZE, STATUS_WRERROR, __FILE__, __LINE__);
}

//读data文件
static inline const void* FILE_READVALUE(BPTree* bt, off_t valPointer)
{
	lseek(bt->fdData, valPointer, SEEK_SET);
	CONDCHECK(read(bt->fdData, bt->tmpRet, VALSIZE) == (ssize_t)VALSIZE, STATUS_RDERROR, __FILE__, __LINE__);
	return bt->tmpRet;
}

//释放data文件
static inline void FILE_VALUERELEASE(BPTree* bt, off_t valPointer)
{
	long long page = valPointer / DATA_PAGESIZE;
	off_t pagePointer = page * DATA_PAGESIZE;
	POINTCREATE_INIT(char*, dataStr, char, DATA_PAGESIZE);
	lseek(bt->fdData, pagePointer, SEEK_SET);
	CONDCHECK(read(bt->fdData, dataStr, DATA_PAGESIZE) == (ssize_t)DATA_PAGESIZE, STATUS_RDERROR, __FILE__, __LINE__);
	long long idx = (valPointer - pagePointer - sizeof(__value_size_t) - DATAPAGEBITBYTES) / VALSIZE + 1;
	long long bytes = idx / 8 + 1;
	long long bits = idx % 8;
	if (!bits){
		bytes--;
		bits = 8;
	}
	unsigned char* tmp = dataStr + sizeof(__value_size_t) + bytes - 1;
	*tmp ^= 1 << (8 - bits);
	__value_size_t size;
	memcpy(&size, dataStr, sizeof(__value_size_t));
	if (--size){
		memcpy(dataStr, &size, sizeof(__value_size_t));
		lseek(bt->fdData, pagePointer, SEEK_SET);
		CONDCHECK(write(bt->fdData, dataStr, DATA_PAGESIZE) == (ssize_t)DATA_PAGESIZE, STATUS_WRERROR, __FILE__, __LINE__);
	}
	else{
		__release_value_address(bt, pagePointer);
		CONDCHECK(fallocate(bt->fdData, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, pagePointer, DATA_PAGESIZE) >= 0, STATUS_FALLOCATEERROR, __FILE__, __LINE__);
	}
	FREE(dataStr);
}

//写index文件
//存储顺序:isLeaf-size-pKey-childPointers
static inline void FILE_KEYNODEWRITE(BPTree* bt, BPNode* node)
{	
	if (node->size == 0 || node->size > KEYNODECOUNT_MAX)//结点最终会释放,合并或分解
		return;
	if (node->selfPointer == -1)
		node->selfPointer = __apply_valid_keynode_address(bt);
	char* tmpWriteStr = bt->tmpWriteStr;
	memcpy(tmpWriteStr, &(node->isLeaf), sizeof(node->isLeaf));
	ssize_t length = sizeof(node->isLeaf);
	memcpy(tmpWriteStr + length, &(node->size), sizeof(node->size));
	length += sizeof(node->size);
	memcpy(tmpWriteStr + length, node->pKey, KEYSIZE * node->size);
	length += KEYSIZE * node->size;
	memcpy(tmpWriteStr + length, node->childPointers, sizeof(off_t) * (node->size + 1));
	length += sizeof(off_t) * (node->size + 1);

	lseek(bt->fdIndex, node->selfPointer, SEEK_SET);
	CONDCHECK(write(bt->fdIndex, tmpWriteStr, length) == length, STATUS_WRERROR, __FILE__, __LINE__);
}

//存储的size必然大于等于1
static inline void FILE_READNODE(BPTree* bt, off_t pointer, BPNode* node)
{
	node->selfPointer = pointer;
	lseek(bt->fdIndex, pointer, SEEK_SET);
	ssize_t _size = sizeof(node->isLeaf);
	CONDCHECK(read(bt->fdIndex, &(node->isLeaf), _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
	_size = sizeof(node->size);
	CONDCHECK(read(bt->fdIndex, &(node->size), _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
	_size = node->size * KEYSIZE;
	CONDCHECK(read(bt->fdIndex, node->pKey, _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
	_size = (node->size + 1) * sizeof(off_t);
	CONDCHECK(read(bt->fdIndex, node->childPointers, _size) == _size, STATUS_RDERROR, __FILE__, __LINE__);
}

//释放index文件对应addr空间
static inline void FILE_KEYNODERELEASE(BPTree* bt, BPNode* node)
{
	__release_keynode_address(bt, node->selfPointer);
	CONDCHECK(fallocate(bt->fdIndex, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, node->selfPointer, INDEX_PAGESIZE) >= 0, STATUS_FALLOCATEERROR, __FILE__, __LINE__);
	node->selfPointer = -1;
}

//写元信息
static inline void FILE_WRITEMETAHEADER(BPTree* bt)
{
	memcpy(bt->metaMap, &(bt->meta), sizeof(BPMetaNode));
}

/*结点某位置及之后数据往后移动一格*/
static inline void MOVEBACKONESTEP(BPTree* bt, BPNode* node, __keynode_size_t loc)
{
	if (loc < node->size){
		memmove(node->pKey + KEYSIZE * (loc + 1), node->pKey + KEYSIZE * loc, KEYSIZE * (node->size - loc));
		memmove(node->childPointers + loc + 1, node->childPointers + loc, sizeof(off_t) * (node->size - loc + 1));
		return;
	}
	if (ISLEAF(node))//移动next指针
		node->childPointers[loc + 1] = node->childPointers[loc];
}

/*结点某位置及之后数据往前移动一格*/
static inline void MOVEFORWARDONESTEP(BPTree* bt, BPNode* node, __keynode_size_t loc)
{
	if (loc < node->size){
		memmove(node->pKey + KEYSIZE * (loc - 1), node->pKey + KEYSIZE * loc, KEYSIZE * (node->size - loc));
		memmove(node->childPointers + loc - 1, node->childPointers + loc, sizeof(off_t) * (node->size - loc + 1));
		return;
	}
	if (ISLEAF(node))//移动next指针
		node->childPointers[loc - 1] = node->childPointers[loc];
}

static BPTree* create(size_t keySize, size_t valSize, BPKeyCompareFuncT equalFunc, BPKeyCompareFuncT lessFunc, const char* filePath)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR, __FILE__, __LINE__);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);

	POINTCREATE_INIT(BPTree*, bt, BPTree, sizeof(BPTree));
	GETALLFILENAME(filePath);
	int flag = access(bt->META_FILENAME, F_OK);
	CONDCHECK(flag == access(bt->INDEX_FILENAME, F_OK), STATUS_FILEUNMATCHED, __FILE__, __LINE__);
	CONDCHECK(flag == access(bt->DATA_FILENAME, F_OK), STATUS_FILEUNMATCHED, __FILE__, __LINE__);
	CONDCHECK((bt->fdMeta = open(bt->META_FILENAME, O_RDWR | O_CREAT, 777)) > 0, STATUS_FDERROR, __FILE__, __LINE__);
	CONDCHECK((bt->fdIndex = open(bt->INDEX_FILENAME, O_RDWR | O_CREAT, 777)) > 0, STATUS_FDERROR, __FILE__, __LINE__);
	CONDCHECK((bt->fdData = open(bt->DATA_FILENAME, O_RDWR | O_CREAT, 777)) > 0, STATUS_FDERROR, __FILE__, __LINE__);
	CONDCHECK((bt->metaMap = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, bt->fdMeta, 0)) != MAP_FAILED, STATUS_MMAPFAILED, __FILE__, __LINE__);
	if (flag == 0){//文件存在,代表已经创建
		memcpy(&(bt->meta), bt->metaMap, sizeof(BPMetaNode));
		CONDCHECK(KEYSIZE == keySize && VALSIZE == valSize, STATUS_SIZEERROR, __FILE__, __LINE__);
		CONDCHECK(META_PAGESIZE == getpagesize() && INDEX_PAGESIZE == getpagesize() * INDEX_PAGE_RATE && DATA_PAGESIZE == getpagesize() * DATA_PAGE_RATE, STATUS_SIZEERROR, __FILE__, __LINE__);
	}
	else{//元信息初始化
		//键值大小
		KEYSIZE = keySize;
		VALSIZE = valSize;
		//3个文件页大小
		META_PAGESIZE = getpagesize();
		INDEX_PAGESIZE = getpagesize() * INDEX_PAGE_RATE;
		DATA_PAGESIZE = getpagesize() * DATA_PAGE_RATE;
		//指针初始化
		ROOTPOINTER = FIRSTPOINTER = -1;
		//数据页最大行数(valSize * x + x / 8 = DATA_PAGESIZE - sizeof(__value_size_t))
		VALUECOUNT_MAX = (DATA_PAGESIZE - sizeof(__value_size_t)) * 8 / (8 * valSize + 1);
		CONDCHECK(VALUECOUNT_MAX > 0, STATUS_SIZEERROR, __FILE__, __LINE__);
		DATAPAGEBITBYTES = ceil(VALUECOUNT_MAX / 8.0f);
		if (DATAPAGEBITBYTES + valSize * VALUECOUNT_MAX + sizeof(__value_size_t) > (size_t)DATA_PAGESIZE){
			VALUECOUNT_MAX--;
			DATAPAGEBITBYTES++;
		}
		CONDCHECK(VALUECOUNT_MAX > 0, STATUS_SIZEERROR, __FILE__, __LINE__);
		//结点最大最小关键字数
		KEYNODECOUNT_MAX = (INDEX_PAGESIZE - sizeof(off_t) - sizeof(__keynode_size_t) - sizeof(unsigned char)) / (keySize + sizeof(off_t));
		__keynode_size_t t = (KEYNODECOUNT_MAX + 1) / 2;
		CONDCHECK(t >= 2, STATUS_DEERROR, __FILE__, __LINE__);
		KEYNODECOUNT_MIN = t;
		//元文件位图分界
		INDEXBITMAPEDGE = META_PAGESIZE / ((KEYNODECOUNT_MAX + KEYNODECOUNT_MIN) / 2 + VALUECOUNT_MAX) * VALUECOUNT_MAX;
		//元文件扩展
		FILE_EXTENDMETA(bt);
	}

	POINTCREATE(EMPTYDEF, bt->tmpRet, void, valSize);
	POINTCREATE(EMPTYDEF, bt->tmpWriteStr, char, INDEX_PAGESIZE);
	bt->equalFunc = equalFunc;
	bt->lessFunc = lessFunc;
	return bt;
}

static inline void destroy(BPTree** sbt)
{
	FILE_WRITEMETAHEADER(*sbt);
	munmap((*sbt)->metaMap, MMAP_SIZE);

	close((*sbt)->fdMeta);
	fsync((*sbt)->fdIndex);
	close((*sbt)->fdIndex);
	fsync((*sbt)->fdData);
	close((*sbt)->fdData);

	FREE((*sbt)->tmpRet);
	FREE((*sbt)->tmpWriteStr);
	FREE(*sbt);
}

static void traverse(BPTree* bt, BPForEachFuncT func)
{
	if (FIRSTPOINTER == -1)
		return;
	BPNode* node = NEWBPNODE(bt);
	off_t pointer = FIRSTPOINTER;
	while(true){
		FILE_READNODE(bt, pointer, node);
		for (__keynode_size_t i = 0; i < node->size; i++)
			func(node->pKey + KEYSIZE * i, FILE_READVALUE(bt, node->childPointers[i]));
		pointer = node->childPointers[node->size];
		if (pointer == -1)
			break;
	}
	RELEASEBPNODE(&node);
}

static inline __keynode_size_t __find_key_sw_loc(BPTree* bt, BPNode* node, const void* pKey)
{
	__keynode_size_t left = 0, right = node->size - 1;
	while(true){
		__keynode_size_t mid = (left + right) / 2;
		if (bt->equalFunc(node->pKey + mid * KEYSIZE, pKey))
			return mid;
		if (bt->lessFunc(node->pKey + mid * KEYSIZE, pKey)){
			if (left == mid)
				return left;
			right = mid;
		}
		else{
			if (left == mid)
				return right;
			left = mid + 1;
		}
	}
}

static inline BPNode* SPLITNODE(BPTree* bt, BPNode* node)
{
	BPNode* newNode = NEWBPNODE(bt);
	__keynode_size_t edge = node->size / 2;
	memcpy(newNode->pKey, node->pKey + KEYSIZE * edge, KEYSIZE * (node->size - edge));
	memcpy(newNode->childPointers, node->childPointers + edge, sizeof(off_t) * (node->size - edge + 1));
	newNode->size = node->size - edge;
	newNode->isLeaf = node->isLeaf;
	FILE_KEYNODEWRITE(bt, newNode);
	node->childPointers[edge] = newNode->selfPointer;
	node->size = edge;
	FILE_KEYNODEWRITE(bt, node);
	return newNode;
}

static inline void __do_balance_insert(BPTree* bt, BPNode** nnode, SqStack* stack_node, SqStack* stack_loc)
{
	while(true){
		if ((*nnode)->size <= KEYNODECOUNT_MAX)
			break;
		BPNode* spl = SPLITNODE(bt, *nnode);
		if (SqStack().empty(stack_node)){//新根节点
			BPNode* newRoot = NEWBPNODE(bt);
			memcpy(newRoot->pKey, (*nnode)->pKey + KEYSIZE * ((*nnode)->size - 1), KEYSIZE);
			newRoot->childPointers[0] = (*nnode)->selfPointer;
			memcpy(newRoot->pKey + KEYSIZE, spl->pKey + KEYSIZE * (spl->size - 1), KEYSIZE);
			newRoot->childPointers[1] = spl->selfPointer;
			newRoot->size = 2;
			FILE_KEYNODEWRITE(bt, newRoot);
			ROOTPOINTER = newRoot->selfPointer;
			RELEASEBPNODE(&spl);
			RELEASEBPNODE(nnode);
			*nnode = newRoot;
			break;
		}
		BPNode* parent = TOCONSTANT(BPNode*, SqStack().pop(stack_node));
		__keynode_size_t loc = TOCONSTANT(__keynode_size_t, SqStack().pop(stack_loc));
		parent->childPointers[loc] = spl->selfPointer;
		MOVEBACKONESTEP(bt, parent, loc);
		memcpy(parent->pKey + KEYSIZE * loc, (*nnode)->pKey + KEYSIZE * ((*nnode)->size - 1), KEYSIZE);
		parent->childPointers[loc] = (*nnode)->selfPointer;
		parent->size++;
		FILE_KEYNODEWRITE(bt, parent);
		RELEASEBPNODE(&spl);
		RELEASEBPNODE(nnode);
		*nnode = parent;
	}
}

static void insert(BPTree* bt, const void* pKey, const void* pValue)
{
	//空树
	if (ROOTPOINTER == -1){
		BPNode* node = NEWBPNODE(bt);
		memcpy(node->pKey, pKey, KEYSIZE);
		node->childPointers[0] = FILE_VALUEWRITE(bt, pValue);
		node->childPointers[1] = -1;
		node->isLeaf = 1;
		node->size = 1;
		bt->meta.rows = 1;
		FILE_KEYNODEWRITE(bt, node);
		ROOTPOINTER = node->selfPointer;
		FIRSTPOINTER = node->selfPointer;
		RELEASEBPNODE(&node);
		return;
	}
	//找到叶子结点并记录路径
	SqStack* stack_node = SqStack().create(sizeof(BPNode*), NULL);
	SqStack* stack_loc = SqStack().create(sizeof(__keynode_size_t), NULL);
	off_t pointer = ROOTPOINTER;
	BPNode* node = NEWBPNODE(bt);
	FILE_READNODE(bt, pointer, node);
	bool updateMax = (!bt->equalFunc(node->pKey + (node->size - 1) * KEYSIZE, pKey))\
		&& (!bt->lessFunc(node->pKey + (node->size - 1) * KEYSIZE, pKey));
	__keynode_size_t loc;
	while(true){
		if (updateMax)
			loc = node->size - 1;
		else
			loc = __find_key_sw_loc(bt, node, pKey);
		if (ISLEAF(node))
			break;
		SqStack().push(stack_node, &node);
		SqStack().push(stack_loc, &loc);
		if (updateMax){//更新树最大关键字
			memcpy(node->pKey + loc * KEYSIZE, pKey, KEYSIZE);
			FILE_KEYNODEWRITE(bt, node);
		}
		BPNode* child = NEWBPNODE(bt);
		FILE_READNODE(bt, node->childPointers[loc], child);
		node = child;
	}
	//插入
	if (bt->equalFunc(node->pKey + loc * KEYSIZE, pKey))
		FILE_VALUECOVER(bt, node->childPointers[loc], pValue);
	else{
		if (!bt->lessFunc(node->pKey + loc * KEYSIZE, pKey))
			loc++;
		MOVEBACKONESTEP(bt, node, loc);
		memcpy(node->pKey + KEYSIZE * loc, pKey, KEYSIZE);
		node->childPointers[loc] = FILE_VALUEWRITE(bt, pValue);
		node->size++;
		FILE_KEYNODEWRITE(bt, node);
		__do_balance_insert(bt, &node, stack_node, stack_loc);
		bt->meta.rows++;
	}
	//释放空间
	RELEASEBPNODE(&node);
	SQSTACK_FOREACH(stack_node, BPNode*, {
		RELEASEBPNODE(&value);
	});
	SqStack().destroy(&stack_node);
	SqStack().destroy(&stack_loc);
}

static inline void __do_balance_erase(BPTree* bt, BPNode** nnode, SqStack* stack_node, SqStack* stack_loc)
{
	BPNode* sbl = NULL;
	while(true){
		//size满足,结束判断
		if ((*nnode)->size >= KEYNODECOUNT_MIN)
			break;
		//根结点
		if (SqStack().empty(stack_node)){
			if (ISLEAF(*nnode)){
				if ((*nnode)->size == 0){
					ROOTPOINTER = FIRSTPOINTER = -1;
					FILE_KEYNODERELEASE(bt, *nnode);
				}
				break;
			}
			if ((*nnode)->size < 2){
				ROOTPOINTER = (*nnode)->childPointers[0];
				FILE_KEYNODERELEASE(bt, *nnode);
			}
			break;
		}
		//非根结点
		if (!sbl) sbl = NEWBPNODE(bt);
		BPNode* parent = TOCONSTANT(BPNode*, SqStack().pop(stack_node));
		__keynode_size_t leftLoc, loc = TOCONSTANT(__keynode_size_t, SqStack().pop(stack_loc));
		BPNode* left = NULL, *right = NULL;
		bool ret = false;
		//向左兄弟借
		if (loc > 0){
			leftLoc = loc - 1;
			FILE_READNODE(bt, parent->childPointers[leftLoc], sbl);
			if (sbl->size > KEYNODECOUNT_MIN){
				MOVEBACKONESTEP(bt, *nnode, 0);
				memcpy((*nnode)->pKey, sbl->pKey + KEYSIZE * (sbl->size - 1), KEYSIZE);
				(*nnode)->childPointers[0] = sbl->childPointers[sbl->size - 1];
				(*nnode)->size++;
				sbl->childPointers[sbl->size - 1] = sbl->childPointers[sbl->size];
				sbl->size--;
				memcpy(parent->pKey + KEYSIZE * leftLoc, sbl->pKey + KEYSIZE * (sbl->size - 1), KEYSIZE);
				FILE_KEYNODEWRITE(bt, *nnode);
				FILE_KEYNODEWRITE(bt, sbl);
				FILE_KEYNODEWRITE(bt, parent);
				RELEASEBPNODE(nnode);
				*nnode = parent;
				ret = true;
				break;
			}
			else{
				left = sbl;
				right = *nnode;
			}
		}
		//向右兄弟借
		if (!ret && loc + 1 != parent->size){
			leftLoc = loc;
			FILE_READNODE(bt, parent->childPointers[leftLoc + 1], sbl);
			if (sbl->size > KEYNODECOUNT_MIN){
				(*nnode)->childPointers[(*nnode)->size + 1] = (*nnode)->childPointers[(*nnode)->size];
				memcpy((*nnode)->pKey + KEYSIZE * (*nnode)->size, sbl->pKey, KEYSIZE);
				(*nnode)->childPointers[(*nnode)->size] = sbl->childPointers[0];
				(*nnode)->size++;
				MOVEFORWARDONESTEP(bt, sbl, 1);
				sbl->size--;
				memcpy(parent->pKey + KEYSIZE * leftLoc, (*nnode)->pKey + KEYSIZE * ((*nnode)->size - 1), KEYSIZE);
				FILE_KEYNODEWRITE(bt, *nnode);
				FILE_KEYNODEWRITE(bt, sbl);
				FILE_KEYNODEWRITE(bt, parent);
				RELEASEBPNODE(nnode);
				*nnode = parent;
				ret = true;
				break;
			}
			else{
				left = *nnode;
				right = sbl;
			}
		}
		//合并
		if (!ret){
			memcpy(left->pKey + KEYSIZE * left->size, right->pKey, KEYSIZE * right->size);
			memcpy(left->childPointers + left->size, right->childPointers, sizeof(off_t) * (right->size + 1));
			left->size += right->size;
			MOVEFORWARDONESTEP(bt, parent, leftLoc + 1);
			parent->childPointers[leftLoc] = left->selfPointer;
			parent->size--;
			FILE_KEYNODEWRITE(bt, left);
			FILE_KEYNODERELEASE(bt, right);
			FILE_KEYNODEWRITE(bt, parent);
			RELEASEBPNODE(nnode);
		}
		//下一轮判断
		*nnode = parent;
	}
	if (sbl) RELEASEBPNODE(&sbl);
}

static void erase(BPTree* bt, const void* pKey)
{
	//空树
	if (ROOTPOINTER == -1)
		return;
	//找到叶子结点并记录路径
	SqStack* stack_node = SqStack().create(sizeof(BPNode*), NULL);
	SqStack* stack_loc = SqStack().create(sizeof(__keynode_size_t), NULL);
	off_t pointer = ROOTPOINTER;
	BPNode* node = NULL;
	__keynode_size_t loc;
	while(true){
		node = NEWBPNODE(bt);
		FILE_READNODE(bt, pointer, node);
		loc = __find_key_sw_loc(bt, node, pKey);
		if (ISLEAF(node))
			break;
		SqStack().push(stack_node, &node);
		SqStack().push(stack_loc, &loc);
		pointer = node->childPointers[loc];
	}
	//删除
	if (bt->equalFunc(node->pKey + KEYSIZE * loc, pKey)){
		FILE_VALUERELEASE(bt, node->childPointers[loc]);
		MOVEFORWARDONESTEP(bt, node, loc + 1);
		node->size--;
		FILE_KEYNODEWRITE(bt, node);
		if (node->size == loc--){
			const void* keyMax = node->pKey + KEYSIZE * loc;
			SQSTACK_FOREACH_REVERSE(stack_node, BPNode*, {
				__keynode_size_t valueLoc = TOCONSTANT(__keynode_size_t, SqStack().at(stack_loc, key));
				memcpy(value->pKey + KEYSIZE * valueLoc, keyMax, KEYSIZE);
				FILE_KEYNODEWRITE(bt, value);
				if (valueLoc + 1 != value->size)
					break;
			});
		}
		__do_balance_erase(bt, &node, stack_node, stack_loc);
		bt->meta.rows--;
	}
	//释放空间
	RELEASEBPNODE(&node);
	SQSTACK_FOREACH(stack_node, BPNode*, {
		RELEASEBPNODE(&value);
	});
	SqStack().destroy(&stack_node);
	SqStack().destroy(&stack_loc);
}

static const void* at(BPTree* bt, const void* pKey)
{
	if (ROOTPOINTER == -1)
		return NULL;
	BPNode* node = NEWBPNODE(bt);
	off_t pointer = ROOTPOINTER;
	do{
		FILE_READNODE(bt, pointer, node);
		__keynode_size_t loc = __find_key_sw_loc(bt, node, pKey);
		if (ISLEAF(node)){
			if (bt->equalFunc(node->pKey + KEYSIZE * loc, pKey)){
				FILE_READVALUE(bt, node->childPointers[loc]);
				RELEASEBPNODE(&node);
				return bt->tmpRet;
			}
			RELEASEBPNODE(&node);
			return NULL;
		}
		pointer = node->childPointers[loc];
	}while(true);
}

static inline void change(BPTree* bt, const void* pKey, const void* pValue)
{
	insert(bt, pKey, pValue);
}

static unsigned long long rows(BPTree* bt)
{
	return bt->meta.rows;
}

static BPTree* rebuild(BPTree** sbt)
{
	BPTree* bt = *sbt;
	if (FIRSTPOINTER == -1)
		return bt;
	char META_FILENAME[4096];
	char INDEX_FILENAME[4096];
	char DATA_FILENAME[4096];
	memcpy(META_FILENAME, bt->META_FILENAME, 4096);
	memcpy(INDEX_FILENAME, bt->INDEX_FILENAME, 4096);
	memcpy(DATA_FILENAME, bt->DATA_FILENAME, 4096);
	unsigned long long __rows = rows(bt);
	unsigned long long __t_rows = lseek(bt->fdData, 0, SEEK_END) / DATA_PAGESIZE * VALUECOUNT_MAX;
	char tmpPath[4096];
	memcpy(tmpPath, META_FILENAME, 4096);
	memcpy(tmpPath + strlen(tmpPath) - strlen(".ccMeta"), ".tmp\0", 5);

	BPTree* newBt = create(KEYSIZE, VALSIZE, bt->equalFunc, bt->lessFunc, tmpPath);
	BPNode* node = NEWBPNODE(bt);
	off_t pointer = FIRSTPOINTER;
	while(true){
		FILE_READNODE(bt, pointer, node);
		for (__keynode_size_t i = 0; i < node->size; i++)
			insert(newBt, node->pKey + KEYSIZE * i, FILE_READVALUE(bt, node->childPointers[i]));
		pointer = node->childPointers[node->size];
		if (pointer == -1)
			break;
	}
	RELEASEBPNODE(&node);
	destroy(sbt);
	CONDCHECK(unlink(META_FILENAME) >= 0, STATUS_UNLINKFAILED, __FILE__, __LINE__);
	CONDCHECK(unlink(INDEX_FILENAME) >= 0, STATUS_UNLINKFAILED, __FILE__, __LINE__);
	CONDCHECK(unlink(DATA_FILENAME) >= 0, STATUS_UNLINKFAILED, __FILE__, __LINE__);
	CONDCHECK(rename(newBt->META_FILENAME, META_FILENAME) >= 0, STATUS_RENAMEFAILED, __FILE__, __LINE__);
	CONDCHECK(rename(newBt->INDEX_FILENAME, INDEX_FILENAME) >= 0, STATUS_RENAMEFAILED, __FILE__, __LINE__);
	CONDCHECK(rename(newBt->DATA_FILENAME, DATA_FILENAME) >= 0, STATUS_RENAMEFAILED, __FILE__, __LINE__);
	memcpy(newBt->META_FILENAME, META_FILENAME, 4096);
	memcpy(newBt->INDEX_FILENAME, INDEX_FILENAME, 4096);
	memcpy(newBt->DATA_FILENAME, DATA_FILENAME, 4096);

	unsigned long long _rows = rows(newBt);
	unsigned long long _t_rows = lseek(newBt->fdData, 0, SEEK_END) / DATA_PAGESIZE * VALUECOUNT_MAX;
	printf("before rebuild, used rows:%ld, total rows:%ld, usage:%.2f%\n", __rows, __t_rows, __rows * 100.0f / __t_rows);
	printf("after rebuild, used rows:%ld, total rows:%ld, usage:%.2f%\n", _rows, _t_rows, _rows * 100.0f / _t_rows);

	return newBt;
}

inline const BPTreeOp* GetBPTreeOpStruct()
{
	static const BPTreeOp OpList = {
		.create = create,
		.destroy = destroy,
		.traverse = traverse,
		.insert = insert,
		.erase = erase,
		.at = at,
		.change = change,
		.rows = rows,
		.rebuild = rebuild,
	};
	return &OpList;
}