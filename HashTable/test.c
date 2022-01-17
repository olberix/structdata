#include "HashTable.h"

void display(const void* key, void* val, void* args)
{
	*(int*)val *= 10;
	printf("key:%d\tval:%d\n", *(int*)key, *(int*)val);
}

int main(int argc, char const *argv[])
{
	HashTable* table = HashTable().create(sizeof(int), sizeof(int), default_hash_func_int32, default_less_func_int32, default_equal_func_int32);
	int range = 100;
	for (int i = 0; i < range; i++){
		int key = i;
		int value = i * 10;
		HashTable().insert(table, &key, &value);
	}

	for (int i = 0; i < range; i++){
		int key = i;
		const void* ret = HashTable().at(table, &key);
		int _vv = TOCONSTANT(int, ret);
		printf("%d----\n", _vv);
	}
	puts("++++++++++++++");
	HashTable().for_each(table, display, NULL);
	puts("++++++++++++++");
	HashTable().for_each(table, display, NULL);
	HashTable().destroy(&table);
	return 0;
}