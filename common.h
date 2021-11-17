#ifndef _HEAD_H_
#define _HEAD_H_
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>

/*
*所有实现都不是线程安全
*折腾了一下午还是不实现深拷贝了,在c的封装中实现深拷贝需要函数和返回值的各种协调,赋值也麻烦,还不如直接在应用层手动操作来的方便
*/

enum StatusCode{
	STATUS_INVALIDINDEX = 0,
	STATUS_OVERFLOW = 1,
	STATUS_NOELEM = 2,
	STATUS_SIZEERROR = 3,
	STATUS_NULLFUNC = 4,
	STATUS_DEERROR = 5,
	STATUS_OFFSETERROR = 6,
	STATUS_FDERROR = 7,
	STATUS_RDERROR = 8,
	STATUS_WRERROR = 9,
};

static const char* errStr[] = {
	"invalid index", "overflow", "no elem", "elem size error", "null function", "degree error", "file descriptor error",
	"read error", "write error",
};

#define FREE(p) do{\
	free(p);\
	(p) = NULL;\
}while(0)

/*-DDEBUG -g -rdynamic*/
#ifdef DEBUG
#define CONDCHECK(con, code) do{\
	if (!(con)){\
		if ((code) <= 6)\
			fprintf(stderr, "%s\n", errStr[code]);\
		else\
			perror(errStr[code]);\
		void *array[20];\
		size_t traceSize = backtrace(array, 20);\
		char **strings = backtrace_symbols(array, traceSize);\
		if (strings){\
			for (size_t i = 0; i < traceSize; i++)\
				printf("%s\n", strings[i]);\
		}\
		else{\
			fprintf(stderr, "%s\n", "backtrace_symbols error");\
		}\
		FREE(strings);\
	}\
}while (0)
#else
#define CONDCHECK(con, code) do{\
	if (!(con)){\
		fprintf(stderr, "%s\n", errStr[code]);\
	}\
}while (0)
#endif

#define ROOTCHECK(root) do{\
	if (!(root))\
		return;\
}while(0)

#define RELEASENODE(node)\
	FREE((node)->pKey);\
	FREE((node)->pValue);\
	FREE(node);

#define EMPTYDEF
#define POINTCREATE(def, point, type, size)\
	def point = (type*)malloc(size);\
	CONDCHECK(point, STATUS_OVERFLOW);

#define POINTCREATE_INIT(def, point, type, size)\
	def point = (type*)malloc(size);\
	CONDCHECK(point, STATUS_OVERFLOW);\
	memset(point, 0, size);

#define TOCONSTANT(type, point) (*(type*)point)

#endif