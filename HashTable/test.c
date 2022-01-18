#include "HashTable.h"

void display(const void* key, void* val, void* args)
{
	*(int*)val *= 10;
	printf("key:%d\tval:%d\n", *(int*)key, *(int*)val);
}

int main(int argc, char const *argv[])
{
	HashTable* table = HashTable().create(sizeof(int), sizeof(int), default_hash_func_int32, default_less_func_int32, default_equal_func_int32);
	int range = 10000000;
	int times = 10;
	for (int i = 0; i < times; i++){
		for (int j = 0; j < range; j++){
			int key = j;
			int value = j * 10;
			HashTable().insert(table, &key, &value);
		}
		if (i + 1 != times){
			if (i % 7 != 0){
				if (i % 3 == 0)
					for (int j = 0; j < range / 2; j++){
						int key = j;
						HashTable().erase(table, &key);
					}
				else
					for (int j = 0; j < range; j += 2){
						int key = j;
						HashTable().erase(table, &key);
					}
			}
			else
				HashTable().clear(table);
		}
		printf("%d done-----------\n", i);
	}
	puts("check begin---------------------------");
	for (int j = 0; j < range; j++){
		int key = j;
		const void* ret = HashTable().at(table, &key);
		if (!ret){
			printf("error------%d\n", key);
			exit(-1);
		}
		int val = TOCONSTANT(int, ret);
		if (val != key * 10){
			printf("unequal------%d\n", key);
			exit(-1);
		}
	}
	puts("check ok---------------------------");

	printf("size:%lld------------------\n", HashTable().size(table));
	HashTable().clear(table);
	range = 10;
	for (int j = 0; j < range; j++){
		int key = j;
		int value = j * 10;
		HashTable().insert(table, &key, &value);
	}
	puts("++++++++++++++++++++++++++++++++++");
	HashTable().for_each(table, display, NULL);
	puts("++++++++++++++");
	HashTable().for_each(table, display, NULL);

	printf("size:%lld------------------\n", HashTable().size(table));

	HashTable().destroy(&table);
	return 0;
}