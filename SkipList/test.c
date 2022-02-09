#include "SkipList.h"
#include <time.h>

void display(size_t index, const void* pKey, void* args)
{
	printf("%d-----\n", *(int*)pKey);
}

int main(int argc, char const *argv[])
{
	// srand(time(NULL));
	SkipList* list = SkipList().create(sizeof(int), default_equal_func_int32, default_less_func_int32);
	
	for (int times = 0; times < 100; times++){
		int key;
		int range = 120000;
		for (int i = 0; i < range; i += 3){
			key = i;
			SkipList().insert(list, &key);
		}
		for (int i = range - 1; i >= 0; i -= 3){
			key = i;
			SkipList().insert(list, &key);
		}
		for (int i = range - 2; i >= range / 2; i -= 3){
			key = i;
			SkipList().insert(list, &key);
		}
		for (int i = 1; i <= range / 2; i += 3){
			key = i;
			SkipList().insert(list, &key);
		}

		if (SkipList().length(list) != (size_t)range){
			puts("error, *************");
			exit(-1);
		}
		for (int i = 0; i < range; i++){
			const void* ret = SkipList().at(list, i);
			int _key = TOCONSTANT(int, ret);
			if (_key != i){
				puts("error-----");
				exit(-1);
			}
			long long idx = SkipList().find(list, &_key);
			if (idx != i){
				puts("error-----++++");
				exit(-1);
			}
		}

		for(int i = 0; i < range; i++){
			if (SkipList().length(list) != 0){
				if (i % 3 == 2){
					int idx = rand() % SkipList().length(list);
					SkipList().erase_loc(list, idx);
				}
				if(i % 3 == 1)
					SkipList().erase(list, &i);
			}
			else
				SkipList().clear(list);
		}

		if (times % 10 == 0)
			printf("++++++++++++++++++++++------%d\n", times);
	}

	SkipList().clear(list);
	SkipList().clear(list);
	SkipList().clear(list);

	int key;
	int range = 510;
	for (int i = 0; i < range; i += 3){
		key = i;
		SkipList().insert(list, &key);
	}
	for (int i = range - 1; i >= 0; i -= 3){
		key = i;
		SkipList().insert(list, &key);
	}
	for (int i = range - 2; i >= range / 2; i -= 3){
		key = i;
		SkipList().insert(list, &key);
	}
	for (int i = 1; i <= range / 2; i += 3){
		key = i;
		SkipList().insert(list, &key);
	}

	for (int i = 0; i <= range / 2; i += 2){
		key = i;
		SkipList().erase(list, &key);
	}
	for (int i = range - 2; i >= range / 2; i -= 2){
		key = i;
		SkipList().erase(list, &key);
	}

	for (int i = 0; i <= range / 2; i+=2){
		// int idx = rand() % SkipList().length(list);
		// SkipList().erase_loc(list, idx);
		SkipList().erase_loc(list, 0);
	}


	puts("=============");
	SkipList().for_each(list, display, NULL);
	puts("=============");
	SkipList().r_for_each(list, display, NULL);

	// for (int i = 0; i < range; i++){
		// const void* ret = SkipList().at(list, i);
		// int _key = TOCONSTANT(int, ret);
		// printf("%d----\n", _key);
		// long long idx = SkipList().find(list, &i);
		// printf("----------%d\n", idx);
	// }

	SkipList().destroy(&list);
	return 0;
}