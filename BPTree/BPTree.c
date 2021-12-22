#include "BPTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "../DlQueue/DlQueue.h"
#include "../SqStack/SqStack.h"

extern int fsync(int);
extern int getpagesize();
#define PAGE_RATE 1 //实际页大小之内存页大小倍数
#define KEYSIZE (bt->meta.keySize)
#define VALSIZE (bt->meta.valSize)
#define META_PAGESIZE (bt->meta.metaPageSize)
#define INDEX_PAGESIZE (bt->meta.indexPageSize)
#define ROOTPOINTER (bt->meta.rootPointer)
#define FIRSTPOINTER (bt->meta.firstPointer)

#define GETALLFILENAME(path)\
	char META_FILENAME[4096], INDEX_FILENAME[4096], DATA_FILENAME[4096];\
	memset(META_FILENAME, 0, 4096);\
	memset(INDEX_FILENAME, 0, 4096);\
	memset(DATA_FILENAME, 0, 4096);\
	strcat(META_FILENAME, path);\
	strcat(META_FILENAME, ".ccMeta");\
	strcat(INDEX_FILENAME, path);\
	strcat(INDEX_FILENAME, ".ccIndex");\
	strcat(DATA_FILENAME, path);\
	strcat(DATA_FILENAME, ".ccData");

static BPTree* create(size_t keySize, size_t valSize, BPKeyCompareFuncT equalFunc, BPKeyCompareFuncT lessFunc, const char* filePath)
{
	CONDCHECK(keySize > 0 && valSize > 0, STATUS_SIZEERROR, __FILE__, __LINE__);
	CONDCHECK(equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	GETALLFILENAME(filePath);
	int flag = access(META_FILENAME, F_OK);
	CONDCHECK(flag == access(INDEX_FILENAME, F_OK), STATUS_FILEUNMATCHED, __FILE__, __LINE__);
	CONDCHECK(flag == access(DATA_FILENAME, F_OK), STATUS_FILEUNMATCHED, __FILE__, __LINE__);
	POINTCREATE_INIT(BPTree*, bt, BPTree, sizeof(BPTree));
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
	__typeof__(((BPTree*)NULL)->maxNC) t = (bt->maxNC + 1) / 2;//最小度数t>=2
	CONDCHECK(t >= 2, STATUS_DEERROR, __FILE__, __LINE__);//t要往下取整(0.5),若t往上取整,结点合成:t-1+t-2+1=2t-2,此时2(t+0.5)-2=2t-1刚好为最大结点数,
	bt->minNC = t - 1;//但结点分解时,即分解成两个t-1,这时候没有多余结点合并到父结点,不符合逻辑
	return bt;//同理t往下取整,合并和分解都符合B树性质,故t往下取整
}