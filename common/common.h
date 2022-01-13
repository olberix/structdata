#ifndef _HEAD_H_
#define _HEAD_H_
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/*
*所有实现都不是线程安全
*折腾了一下午还是不实现深拷贝了,在c的封装中实现深拷贝需要函数和返回值的各种协调,赋值也麻烦,还不如直接在应用层手动操作来的方便
*/

enum StatusCode{
	STATUS_INVALIDINDEX = 0,
	STATUS_OVERFLOW,
	STATUS_NOELEM,
	STATUS_SIZEERROR,
	STATUS_NULLFUNC,
	STATUS_DEERROR,
	STATUS_OFFSETERROR,
	STATUS_FILEUNMATCHED,
	STATUS_MMAPFAILED,

	STATUS_FDERROR,
	STATUS_RDERROR,
	STATUS_WRERROR,
	STATUS_FALLOCATEERROR,
	STATUS_UNLINKFAILED,
	STATUS_RENAMEFAILED,
};

static const char* errStr[] = {
	"invalid index", "overflow", "no elem", "elem size error", "null function", "degree error", "offset error",
	"file unmatched", "mmap failed", "file descriptor error", "read error", "write error", "fallocate error",
	"unlink failed", "rename failed"
};

#define FREE(p) do{\
	free(p);\
	(p) = NULL;\
}while(0)

/*-DDEBUG -g -rdynamic*/
#ifdef DEBUG
#define CONDCHECK(con, code, file, line) do{\
	if (!(con)){\
		printf("%s:%d\n", file, line);\
		if ((code) < STATUS_FDERROR)\
			fprintf(stderr, "%s\n", errStr[code]);\
		else{\
			perror(errStr[code]);\
		}\
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
		exit(-1);\
	}\
}while (0)
#else
#define CONDCHECK(con, code, file, line) do{\
	if (!(con)){\
		printf("%s:%d\n", file, line);\
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
	CONDCHECK(point, STATUS_OVERFLOW, __FILE__, __LINE__);

#define POINTCREATE_INIT(def, point, type, size)\
	POINTCREATE(def, point, type, size);\
	memset(point, 0, size);

#define TOCONSTANT(type, point) (*((type*)(point)))

//默认比较函数
inline bool default_less_func_int8(const void* basis, const void* rhs) { return (char*)basis > (char*)rhs; }
inline bool default_less_func_uint8(const void* basis, const void* rhs) { return (unsigned char*)basis > (unsigned char*)rhs; }
inline bool default_less_func_int16(const void* basis, const void* rhs) { return (short*)basis > (short*)rhs; }
inline bool default_less_func_uint16(const void* basis, const void* rhs) { return (unsigned short*)basis > (unsigned short*)rhs; }
inline bool default_less_func_int32(const void* basis, const void* rhs) { return (int*)basis > (int*)rhs; }
inline bool default_less_func_uint32(const void* basis, const void* rhs) { return (unsigned int*)basis > (unsigned int*)rhs; }
inline bool default_less_func_int64(const void* basis, const void* rhs) { return (long long int*)basis > (long long int*)rhs; }
inline bool default_less_func_uint64(const void* basis, const void* rhs) { return (unsigned long long int*)basis > (unsigned long long int*)rhs; }
inline bool default_less_func_string(const void* basis, const void* rhs) { return strcmp(basis, rhs) > 0; }

inline bool default_equal_func_int8(const void* basis, const void* rhs) { return (char*)basis == (char*)rhs; }
inline bool default_equal_func_uint8(const void* basis, const void* rhs) { return (unsigned char*)basis == (unsigned char*)rhs; }
inline bool default_equal_func_int16(const void* basis, const void* rhs) { return (short*)basis == (short*)rhs; }
inline bool default_equal_func_uint16(const void* basis, const void* rhs) { return (unsigned short*)basis == (unsigned short*)rhs; }
inline bool default_equal_func_int32(const void* basis, const void* rhs) { return (int*)basis == (int*)rhs; }
inline bool default_equal_func_uint32(const void* basis, const void* rhs) { return (unsigned int*)basis == (unsigned int*)rhs; }
inline bool default_equal_func_int64(const void* basis, const void* rhs) { return (long long int*)basis == (long long int*)rhs; }
inline bool default_equal_func_uint64(const void* basis, const void* rhs) { return (unsigned long long int*)basis == (unsigned long long int*)rhs; }
inline bool default_equal_func_string(const void* basis, const void* rhs) { return !strcmp(basis, rhs); }

//默认哈希函数
inline size_t HASH_SEQ(const unsigned char* stream, size_t length)
{
	static_assert(sizeof(size_t) == 8, "plz run with 64-bit machine mode.");
	const size_t seed = 14695981039346656037ULL;
	const size_t factor = 1099511628211ULL;
	size_t baseCode = seed;
	for(size_t next = 0; next < length; ++next){
		baseCode ^= (size_t)stream[next];
		baseCode *= factor;
	}
	baseCode ^= baseCode >> 32;
	return baseCode;
}
inline size_t default_hash_func_int8(const void* key) { return HASH_SEQ(key, sizeof(char)); }
inline size_t default_hash_func_uint8(const void* key) { return HASH_SEQ(key, sizeof(unsigned char)); }
inline size_t default_hash_func_int16(const void* key) { return HASH_SEQ(key, sizeof(short)); }
inline size_t default_hash_func_uint16(const void* key) { return HASH_SEQ(key, sizeof(unsigned short)); }
inline size_t default_hash_func_int32(const void* key) { return HASH_SEQ(key, sizeof(int)); }
inline size_t default_hash_func_uint32(const void* key) { return HASH_SEQ(key, sizeof(unsigned int)); }
inline size_t default_hash_func_int64(const void* key) { return HASH_SEQ(key, sizeof(long long int)); }
inline size_t default_hash_func_uint64(const void* key) { return HASH_SEQ(key, sizeof(unsigned long long int)); }
inline size_t default_hash_func_string(const void* key) { return HASH_SEQ(key, strlen(key)); }

#endif