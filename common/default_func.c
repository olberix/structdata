#include "common.h"

inline bool default_less_func_int8(const void* basis, const void* rhs) { return *((char*)basis) > *((char*)rhs); }
inline bool default_less_func_uint8(const void* basis, const void* rhs) { return *((unsigned char*)basis) > *((unsigned char*)rhs); }
inline bool default_less_func_int16(const void* basis, const void* rhs) { return *((short*)basis) > *((short*)rhs); }
inline bool default_less_func_uint16(const void* basis, const void* rhs) { return *((unsigned short*)basis) > *((unsigned short*)rhs); }
inline bool default_less_func_int32(const void* basis, const void* rhs) { return *((int*)basis) > *((int*)rhs); }
inline bool default_less_func_uint32(const void* basis, const void* rhs) { return *((unsigned int*)basis) > *((unsigned int*)rhs); }
inline bool default_less_func_int64(const void* basis, const void* rhs) { return *((long long int*)basis) > *((long long int*)rhs); }
inline bool default_less_func_uint64(const void* basis, const void* rhs) { return *((unsigned long long int*)basis) > *((unsigned long long int*)rhs); }
inline bool default_less_func_string(const void* basis, const void* rhs) { return strcmp(basis, rhs) > 0; }
inline bool default_equal_func_int8(const void* basis, const void* rhs) { return *((char*)basis) == *((char*)rhs); }
inline bool default_equal_func_uint8(const void* basis, const void* rhs) { return *((unsigned char*)basis) == *((unsigned char*)rhs); }
inline bool default_equal_func_int16(const void* basis, const void* rhs) { return *((short*)basis) == *((short*)rhs); }
inline bool default_equal_func_uint16(const void* basis, const void* rhs) { return *((unsigned short*)basis) == *((unsigned short*)rhs); }
inline bool default_equal_func_int32(const void* basis, const void* rhs) { return *((int*)basis) == *((int*)rhs); }
inline bool default_equal_func_uint32(const void* basis, const void* rhs) { return *((unsigned int*)basis) == *((unsigned int*)rhs); }
inline bool default_equal_func_int64(const void* basis, const void* rhs) { return *((long long int*)basis) == *((long long int*)rhs); }
inline bool default_equal_func_uint64(const void* basis, const void* rhs) { return *((unsigned long long int*)basis) == *((unsigned long long int*)rhs); }
inline bool default_equal_func_string(const void* basis, const void* rhs) { return !strcmp(basis, rhs); }

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
	// return 1;
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