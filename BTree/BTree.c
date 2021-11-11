#include "BTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

extern int fsync(int);
extern size_t getpagesize();

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
	return bt;
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

}

static void traverse(BTree* bt, BForEachFuncT func)
{

}

static void insert(BTree* bt, const void* pKey, const void* pValue)
{

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