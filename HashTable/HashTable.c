#include "HashTable.h"

#define FREEENTRY(entry)\
	FREE((entry).pKey);\
	FREE((entry).pValue);

static HashTable* create(size_t keySize, size_t valSize, HashFuncTT hashFunc, CmnCompareFunc lessFunc, CmnCompareFunc equalFunc)
{
	CONDCHECK(hashFunc && equalFunc && lessFunc, STATUS_NULLFUNC, __FILE__, __LINE__);
	POINTCREATE_INIT(HashTable*, _ht, HashTable, sizeof(HashTable));
	_ht->list = SqList().create(sizeof(HashBucket), &HASHINITSIZE);
	SqList().mem_init(_ht->list);
	POINTCREATE(EMPTYDEF, _ht->tmpRet, void, valSize);
	_ht->keySize = keySize;
	_ht->valSize = valSize;
	_ht->table_size = HASHINITSIZE;
	_ht->hashFunc = hashFunc;
	_ht->lessFunc = lessFunc;
	_ht->equalFunc = equalFunc;
	return _ht;
}

static inline void rb_clear_foreach(const void* key, void* val, void* args)
{
	HashEntry entry = TOCONSTANT(HashEntry, val);
	FREEENTRY(entry);
}

static void clear(HashTable* table)
{
	SQLIST_FOREACH_UNSAFE(table->list, HashBucket, {
		switch(value.type){
			case BUCKETTYPE_NIL:
				break;
			case BUCKETTYPE_ORIGIN:{
				FREEENTRY(value.bucket.entry);
				break;
			}
			case BUCKETTYPE_LIST:{
				DULIST_FOREACH(value.bucket.entry_list, HashEntry, {
					FREEENTRY(value);
				});
				DucList().destroy(&(value.bucket.entry_list));
				break;
			}
			case BUCKETTYPE_RBTREE:{
				RBTree().traverse(value.bucket.entry_tree, rb_clear_foreach, NULL);
				RBTree().destroy(&(value.bucket.entry_tree));
				break;
			}
			default:{
				CONDCHECK(0, STATUS_INVALIDBUCKETTYPE, __FILE__, __LINE__);
				break;
			}
		}
	});
	SqList().mem_init(table->list);
	table->elem_count = 0;
}

static inline void destroy(HashTable** stable)
{
	clear(*stable);
	SqList().destroy(&((*stable)->list));
	FREE((*stable)->tmpRet);
}

typedef struct Hash2RbFA{
	UnorderedForEachFunc_Mutable hash_fe;
	void* args;
}Hash2RbFA;

static inline void rb_traverse_foreach(const void* key, void* val, void* args)
{
	HashEntry entry = TOCONSTANT(HashEntry, val);
	((Hash2RbFA*)args)->hash_fe(key, entry.pValue, ((Hash2RbFA*)args)->args);
}

static void for_each(HashTable* table, UnorderedForEachFunc_Mutable func, void* args)
{
	SQLIST_FOREACH_UNSAFE(table->list, HashBucket, {
		switch(value.type){
			case BUCKETTYPE_NIL:
				break;
			case BUCKETTYPE_ORIGIN:{
				func(value.bucket.entry.pKey, value.bucket.entry.pValue, args);
				break;
			}
			case BUCKETTYPE_LIST:{
				DULIST_FOREACH(value.bucket.entry_list, HashEntry, {
					func(value.pKey, value.pValue, args);
				});
				break;
			}
			case BUCKETTYPE_RBTREE:{
				Hash2RbFA argss;
				argss.hash_fe = func;
				argss.args = args;
				RBTree().traverse(value.bucket.entry_tree, rb_traverse_foreach, &argss);
				break;
			}
			default:{
				CONDCHECK(0, STATUS_INVALIDBUCKETTYPE, __FILE__, __LINE__);
				break;
			}
		}
	});
}

static void __insert(HashTable*, HashEntry);
static inline void rb_rehash_foreach(const void* key, void* val, void* args)
{
	HashEntry entry = TOCONSTANT(HashEntry, val);
	__insert(args, entry);
}

static void __rehash(HashTable* table)
{
	if (table->elem_count < table->table_size)
		return;
	if (table->table_size < 512)
		table->table_size *= 8;
	else
		table->table_size *= 2;
	SqList* __list = table->list;
	table->list = SqList().create(sizeof(HashBucket), &(table->table_size));
	SqList().mem_init(table->list);
	table->elem_count = 0;
	//reinsert
	SQLIST_FOREACH_UNSAFE(__list, HashBucket, {
		switch(value.type){
			case BUCKETTYPE_NIL:
				break;
			case BUCKETTYPE_ORIGIN:{
				__insert(table, value.bucket.entry);
				break;
			}
			case BUCKETTYPE_LIST:{
				DULIST_FOREACH(value.bucket.entry_list, HashEntry, {
					__insert(table, value);
				});
				DucList().destroy(&(value.bucket.entry_list));
				break;
			}
			case BUCKETTYPE_RBTREE:{
				RBTree().traverse(value.bucket.entry_tree, rb_rehash_foreach, table);
				RBTree().destroy(&(value.bucket.entry_tree));
				break;
			}
			default:{
				CONDCHECK(0, STATUS_INVALIDBUCKETTYPE, __FILE__, __LINE__);
				break;
			}
		}
	});
	SqList().destroy(&__list);
}

static inline void __transfrom_into_list(HashBucket* _bkt)
{
	DuCirLinkList* list = DucList().create(sizeof(HashEntry));
	DucList().push_back(list, &(_bkt->bucket.entry));
	_bkt->bucket.entry_list = list;
	_bkt->type = BUCKETTYPE_LIST;
}

static inline bool __transfrom_into_rbtree(HashTable* table, HashBucket* _bkt)
{
	if (DucList().length(_bkt->bucket.entry_list) < TRANSTOTREESIZE)
		return false;
	RBTree* tree = RBTree().create(table->keySize, sizeof(HashEntry), table->equalFunc, table->lessFunc);
	DULIST_FOREACH(_bkt->bucket.entry_list, HashEntry, {
		RBTree().insert(tree, value.pKey, &value);
	});
	DucList().destroy(&(_bkt->bucket.entry_list));
	_bkt->bucket.entry_tree = tree;
	_bkt->type = BUCKETTYPE_RBTREE;
	return true;
}

static void __insert(HashTable* table, HashEntry entry)
{
	size_t hashCode = table->hashFunc(entry.pKey) & (table->table_size - 1);
	HashBucket _bkt = TOCONSTANT(HashBucket, SqList().at_unsafe(table->list, hashCode));
	__HashBucket* bucket = &(_bkt.bucket);
	switch(_bkt.type){
		case BUCKETTYPE_NIL:{
			bucket->entry = entry;
			_bkt.type = BUCKETTYPE_ORIGIN;
			SqList().change_unsafe(table->list, hashCode, &_bkt);
			table->elem_count++;
			break;
		}
		case BUCKETTYPE_ORIGIN:{
			if (table->equalFunc(bucket->entry.pKey, entry.pKey)){
				void* tmp = bucket->entry.pValue;
				bucket->entry.pValue = entry.pValue;
				entry.pValue = tmp;
				FREEENTRY(entry);
			}
			else{
				__transfrom_into_list(&_bkt);
				DucList().push_back(bucket->entry_list, &entry);
				table->elem_count++;
			}
			SqList().change_unsafe(table->list, hashCode, &_bkt);
			break;
		}
		case BUCKETTYPE_LIST:{
			bool rplc = false;
			DULIST_FOREACH(bucket->entry_list, HashEntry, {
				if (table->equalFunc(value.pKey, entry.pKey)){
					void* tmp = value.pValue;
					value.pValue = entry.pValue;
					entry.pValue = tmp;
					DucList().change(bucket->entry_list, key, &value);
					FREEENTRY(entry);
					rplc = true;
					break;
				}
			});
			if (rplc)
				break;
			DucList().push_back(bucket->entry_list, &entry);
			bool ret = __transfrom_into_rbtree(table, &_bkt);
			if (ret)
				SqList().change_unsafe(table->list, hashCode, &_bkt);
			table->elem_count++;
			break;
		}
		case BUCKETTYPE_RBTREE:{
			const void* ret = RBTree().at(bucket->entry_tree, entry.pKey);
			if (ret){
				HashEntry _entry = TOCONSTANT(HashEntry, ret);
				FREEENTRY(_entry);
				RBTree().insert(bucket->entry_tree, entry.pKey, &entry);
			}
			else{
				RBTree().insert(bucket->entry_tree, entry.pKey, &entry);
				table->elem_count++;
			}
			break;
		}
		default:{
			FREEENTRY(entry);
			CONDCHECK(0, STATUS_INVALIDBUCKETTYPE, __FILE__, __LINE__);
			break;
		}
	}
	__rehash(table);
}

static inline void insert(HashTable* table, const void* pKey, const void* pValue)
{
	HashEntry entry;
	POINTCREATE(EMPTYDEF, entry.pKey, void, table->keySize);
	POINTCREATE(EMPTYDEF, entry.pValue, void, table->valSize);
	memcpy(entry.pKey, pKey, table->keySize);
	memcpy(entry.pValue, pValue, table->valSize);
	__insert(table, entry);
}

static const void* erase(HashTable* table, const void* pKey)
{
	size_t hashCode = table->hashFunc(pKey) & (table->table_size - 1);
	HashBucket _bkt = TOCONSTANT(HashBucket, SqList().at_unsafe(table->list, hashCode));
	__HashBucket* bucket = &(_bkt.bucket);
	switch(_bkt.type){
		case BUCKETTYPE_NIL:
			return NULL;
		case BUCKETTYPE_ORIGIN:{
			if (table->equalFunc(bucket->entry.pKey, pKey)){
				memcpy(table->tmpRet, bucket->entry.pValue, table->valSize);
				FREEENTRY(bucket->entry);
				_bkt.type = BUCKETTYPE_NIL;
				SqList().change_unsafe(table->list, hashCode, &_bkt);
				table->elem_count--;
				return table->tmpRet;
			}
			return NULL;
		}
		case BUCKETTYPE_LIST:{
			DULIST_FOREACH(bucket->entry_list, HashEntry, {
				if (table->equalFunc(value.pKey, pKey)){
					memcpy(table->tmpRet, value.pValue, table->valSize);
					FREEENTRY(value);
					DucList().erase(bucket->entry_list, key);
					table->elem_count--;
					return table->tmpRet;
				}
			});
			return NULL;
		}
		case BUCKETTYPE_RBTREE:{
			const void* ret = RBTree().at(bucket->entry_tree, pKey);
			if (!ret)
				return NULL;
			HashEntry entry = TOCONSTANT(HashEntry, ret);
			memcpy(table->tmpRet, entry.pValue, table->valSize);
			FREEENTRY(entry);
			RBTree().erase(bucket->entry_tree, pKey);
			table->elem_count--;
			return table->tmpRet;
		}
		default:{
			CONDCHECK(0, STATUS_INVALIDBUCKETTYPE, __FILE__, __LINE__);
			return NULL;
		}
	}
}

static const void* at(HashTable* table, const void* pKey)
{
	size_t hashCode = table->hashFunc(pKey) & (table->table_size - 1);
	HashBucket _bkt = TOCONSTANT(HashBucket, SqList().at_unsafe(table->list, hashCode));
	switch(_bkt.type){
		case BUCKETTYPE_NIL:
			return NULL;
		case BUCKETTYPE_ORIGIN:{
			if (table->equalFunc(_bkt.bucket.entry.pKey, pKey)){
				memcpy(table->tmpRet, _bkt.bucket.entry.pValue, table->valSize);
				return table->tmpRet;
			}
			return NULL;
		}
		case BUCKETTYPE_LIST:{
			DULIST_FOREACH(_bkt.bucket.entry_list, HashEntry, {
				if (table->equalFunc(value.pKey, pKey)){
					memcpy(table->tmpRet, value.pValue, table->valSize);
					return table->tmpRet;
				}
			});
			return NULL;
		}
		case BUCKETTYPE_RBTREE:{
			const void* ret = RBTree().at(_bkt.bucket.entry_tree, pKey);
			if (!ret)
				return NULL;
			HashEntry entry = TOCONSTANT(HashEntry, ret);
			memcpy(table->tmpRet, entry.pValue, table->valSize);
			return table->tmpRet;
		}
		default:
			return NULL;
	}
}

static inline void change(HashTable* table, const void* pKey, const void* pValue)
{
	insert(table, pKey, pValue);
}

static inline size_t size(HashTable* table)
{
	return table->elem_count;
}

inline const HashTableOp* GetHashTableOpStruct()
{
	static const HashTableOp OpList = {
		.create = create,
		.clear = clear,
		.destroy = destroy,
		.for_each = for_each,
		.insert = insert,
		.erase = erase,
		.at = at,
		.change = change,
		.size = size,
	};
	return &OpList;
}