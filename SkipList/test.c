#include "SkipList.h"

void display(size_t index, const void* pKey, void* args)
{
	printf("%d-----\n", *(int*)pKey);
}

int main(int argc, char const *argv[])
{
	SkipList* list = SkipList().create(sizeof(int), default_equal_func_int32, default_less_func_int32);
	
	int key = 9;
	// SkipList().insert(list, &key);

	// key = 10;
	// SkipList().insert(list, &key);

	// key = 1;
	// SkipList().insert(list, &key);

	// key = 2;
	// SkipList().insert(list, &key);

	// key = 3;
	// SkipList().insert(list, &key);

	// key = 4;
	// SkipList().insert(list, &key);

	// key = 5;
	// SkipList().insert(list, &key);

	// key = 6;
	// SkipList().insert(list, &key);

	
	int range = 100;
	for (int i = 0; i < range; i += 2){
		key = i;
		SkipList().insert(list, &key);
	}
	for (int i = range - 1; i >= 0; i -= 2){
		key = i;
		SkipList().insert(list, &key);
	}


	// puts("=============");
	// SkipList().for_each(list, display, NULL);
	// puts("=============");
	// SkipList().r_for_each(list, display, NULL);

	// for (int i = 0; i < range; i++){
	// 	const void* ret = SkipList().at(list, i);
	// 	int _key = TOCONSTANT(int, ret);
	// 	printf("%d----\n", _key);
	// }

	SkipList().destroy(&list);
	return 0;
}