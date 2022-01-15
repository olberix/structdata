#include "HashTable.h"

#define FREEENTRY(entry)\
	FREE((entry).pKey);\
	FREE((entry).pValue);

static HashTable* create(size_t keySize, size_t valSize, HashCodeFuncT hashFunc, HKeyCompareFuncT lessFunc, HKeyCompareFuncT equalFunc)
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

static void clear(HashTable* table)
{

}

static inline void destroy(HashTable** stable)
{

}

static void for_each(HashTable* table, UnorderedForEachFunc_Const func)
{

}

static inline void rb_rehash_foreach(const void* key, void* val)
{
	HashEntry entry = TOCONSTANT(HashEntry, val);
}

void __insert(HashTable*, HashEntry);
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
	//copy
	SQLIST_FOREACH(__list, HashEntry, {
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
				
				break;
			}
			default:{
				CONDCHECK(0, STATUS_INVALIDBUCKETTYPE, __FILE__, __LINE__);
				break;
			}
		}
	});
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
	DuCirLinkList* tree = RBTree().create(table->keySize, sizeof(HashEntry), table->equalFunc, table->lessFunc);
	DULIST_FOREACH(_bkt->bucket.entry_list, HashEntry, {
		RBTree().insert(tree, value.pKey, value);
	});
	DucList().destroy(&(_bkt->bucket.entry_list));
	_bkt->bucket.entry_tree = tree;
	_bkt->type = BUCKETTYPE_RBTREE;
	return true;
}

static void __insert(HashTable* table, HashEntry entry)
{
	size_t hashCode = table->hashFunc(entry->pKey) & (table->table_size - 1);
	HashBucket _bkt = TOCONSTANT(SqList().at(table->list, hashCode));
	__HashBucket* bucket = &(_bkt.bucket);
	switch(_bkt.type){
		case BUCKETTYPE_NIL:{
			bucket->entry = entry;
			_bkt.type = BUCKETTYPE_ORIGIN;
			SqList().change(table->list, hashCode, &_bkt);
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
			}
			SqList().change(table->list, hashCode, &_bkt);
			table->elem_count++;
			break;
		}
		case BUCKETTYPE_LIST:{
			bool rplc = false;
			DULIST_FOREACH(bucket->entry_list, HashEntry, {
				if (table->equalFunc(value.pKey, pKey)){
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
				SqList().change(table->list, hashCode, &_bkt);
			table->elem_count++;
			break;
		}
		case BUCKETTYPE_RBTREE:{
			size_t _size = RBTree().size(bucket->entry_tree);
			RBTree().insert(bucket->entry_tree, entry.pKey, entry);
			if (_size != RBTree().size(_bkt.bucket.entry_tree))
				table->elem_count++;
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
	POINTCREATE(EMPTYDEF, entry->pKey, void, table->keySize);
	POINTCREATE(EMPTYDEF, entry->pValue, void, table->valSize);
	memcpy(entry->pKey, pKey, table->keySize);
	memcpy(entry->pValue, pValue, table->valSize);
	__insert(table, entry);
}

static void erase(HashTable* table, const void* pKey)
{

}

static const void* at(HashTable* table, const void* pKey)
{

}

static inline void change(HashTable* table, const void* pKey, const void* pValue)
{

}

static inline size_t size(HashTable* table)
{

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