#include "HashTable.h"

int main(int argc, char const *argv[])
{
	HashTable* table = HashTable().create(sizeof(int), sizeof(int), default_hash_func_int32, default_less_func_int32, default_equal_func_int32);
	int key = 10;
	int value = 20;
	HashTable().insert(table, &key, &value);

	void* ret = HashTable().at(table, &key);
	int _vv = TOCONSTANT(int, ret);
	printf("%d----\n", _vv);


	return 0;
}