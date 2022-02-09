#include "HashTable.h"
#include <math.h>

// void display(const void* key, void* val, void* args)
// {
// 	*(int*)val *= 10;
// 	printf("key:%d\tval:%d\n", *(int*)key, *(int*)val);
// }

// int main(int argc, char const *argv[])
// {
// 	HashTable* table = HashTable().create(sizeof(int), sizeof(int), default_hash_func_int32, default_less_func_int32, default_equal_func_int32);
// 	int range = 10000000;
// 	int times = 100;
// 	for (int i = 0; i < times; i++){
// 		for (int j = 0; j < range; j++){
// 			int key = j;
// 			int value = j * 10;
// 			HashTable().insert(table, &key, &value);
// 		}
// 		if (i + 1 != times){
// 			if (i % 7 != 0){
// 				if (i % 3 == 2)
// 					for (int j = 0; j < range / 2; j++){
// 						int key = j;
// 						HashTable().erase(table, &key);
// 					}
// 				else if (i % 3 == 1)
// 					for (int j = 0; j < range; j += 2){
// 						int key = j;
// 						HashTable().erase(table, &key);
// 					}
// 				else
// 					for (int j = range - 1; j >= 0; j -= 2){
// 						int key = j;
// 						HashTable().erase(table, &key);
// 					}
// 			}
// 			else
// 				HashTable().clear(table);
// 		}
// 		printf("%d done-----------\n", i);
// 	}
// 	puts("check begin---------------------------");
// 	for (int j = 0; j < range; j++){
// 		int key = j;
// 		const void* ret = HashTable().at(table, &key);
// 		if (!ret){
// 			printf("error------%d\n", key);
// 			exit(-1);
// 		}
// 		int val = TOCONSTANT(int, ret);
// 		if (val != key * 10){
// 			printf("unequal------%d\n", key);
// 			exit(-1);
// 		}
// 	}
// 	puts("check ok---------------------------");

// 	printf("size:%lld------------------\n", HashTable().size(table));
// 	HashTable().clear(table);
// 	range = 10;
// 	for (int j = 0; j < range; j++){
// 		int key = j;
// 		int value = j * 10;
// 		HashTable().insert(table, &key, &value);
// 	}
// 	puts("++++++++++++++++++++++++++++++++++");
// 	HashTable().for_each(table, display, NULL);
// 	puts("++++++++++++++");
// 	HashTable().for_each(table, display, NULL);

// 	printf("size:%lld------------------\n", HashTable().size(table));

// 	HashTable().destroy(&table);
// 	return 0;
// }

typedef struct TTKEY{
	double a;
	long int b;
	char c;
}TTKEY;

typedef struct TTVALUE{
	long long int a;
	double b;
	char c[256];
}TTVALUE;

void display(const void* key, void* val, void* args)
{
	const TTKEY* _key = key;
	const TTVALUE* _val = val;
	printf("key:%.8lf\t%ld\t%c\tvalue:%lld\t%.8lf\t%s\n", _key->a, _key->b, _key->c, _val->a, _val->b, _val->c);
}

size_t TTKEY_HASH(const void* key){
	const TTKEY* _key = key;
	return default_hash_func_float64(&(_key->a)) * default_hash_func_int32(&(_key->b)) * default_hash_func_int8(&(_key->c));
}

bool ttkey_less_func(const void* lhs, const void* rhs)
{
	const TTKEY* _lhs = lhs;
	const TTKEY* _rhs = rhs;
	if (_lhs->a != _lhs->a)
		return _lhs->a > _rhs->a;
	else{
		if (_lhs->b != _rhs->b)
			return _lhs->b > _rhs->b;
		else
			return _lhs->c - _rhs->c > EPS_FLOAT64;
	}
}

bool ttkey_equal_func(const void* lhs, const void* rhs)
{
	const TTKEY* _lhs = lhs;
	const TTKEY* _rhs = rhs;
	return _lhs->a == _rhs->a && _lhs->b == _lhs->b && fabs(_lhs->c - _lhs->c) <= EPS_FLOAT64;
}

int main(int argc, char const *argv[])
{
	HashTable* table = HashTable().create(sizeof(TTKEY), sizeof(TTVALUE), TTKEY_HASH, ttkey_less_func, ttkey_equal_func);
	TTKEY key;
	TTVALUE value;
	int range = 1000000;
	int times = 100;
	for (int j = 0; j < times; j++){
		for (int i = 0; i < range; i++){
			key.a = i + 0.555555;
			key.b = i;
			value.a = key.b;
			value.b = key.a;
			if (i % 3 == 2){
				key.c = 'a';
				memcpy(value.c, "aaaa\0", 5);
			}
			else if (i % 3 == 1){
				key.c = 'b';
				memcpy(value.c, "bbbb\0", 5);
			}
			else{
				key.c = 'c';
				memcpy(value.c, "cccc\0", 5);
			}

			HashTable().insert(table, &key, &value);
		}
		if (j + 1 != times){
			if (j % 7 != 0){
				if (j % 3 == 2)
					for (int m = 0; m < range / 2; m++){
						key.a = m + 0.555555;
						key.b = m;
						if (m % 3 == 2)
							key.c = 'a';
						else if (m % 3 == 1)
							key.c = 'b';
						else
							key.c = 'c';
						HashTable().erase(table, &key);
					}
				else if (j % 3 == 1)
					for (int m = 0; m < range; m += 2){
						key.a = m + 0.555555;
						key.b = m;
						if (m % 3 == 2)
							key.c = 'a';
						else if (m % 3 == 1)
							key.c = 'b';
						else
							key.c = 'c';
						HashTable().erase(table, &key);
					}
				else
					for (int m = range - 1; m >= 0; m -= 2){
						key.a = m + 0.555555;
						key.b = m;
						if (m % 3 == 2)
							key.c = 'a';
						else if (m % 3 == 1)
							key.c = 'b';
						else
							key.c = 'c';
						HashTable().erase(table, &key);
					}
			}
			else
				HashTable().clear(table);
		}

		printf("times:%d done-----\n", j);
	}
	printf("size:%ld------------------\n", HashTable().size(table));
	puts("check begin---------------------------");
	for (int i = 0; i < range; i++){
		key.a = i + 0.555555;
		key.b = i;
		if (i % 3 == 2)
			key.c = 'a';
		else if (i % 3 == 1)
			key.c = 'b';
		else
			key.c = 'c';
		const void* ret = HashTable().at(table, &key);
		if (!ret){
			printf("can not find expect value------%d\n", i);
			exit(-1);
		}
		TTVALUE val = TOCONSTANT(TTVALUE, ret);
		if (val.a != key.b){
			printf("integer unequal------%d %ld %lld\n", i, key.b, val.a);
			exit(-1);
		}
		if (fabs(key.a-val.b) > EPS_FLOAT64){
			printf("float unequal------%d\n", i);
			exit(-1);
		}
		if (i % 3 == 2){
			if (strncmp(val.c, "aaaa\0", 5) != 0){
				printf("aaaa-unequal------%d-%s\n", i, val.c);
				exit(-1);
			}
		}
		else if (i % 3 == 1){
			if (strncmp(val.c, "bbbb\0", 5) != 0){
				printf("bbbb-unequal------%d-%s\n", i, val.c);
				exit(-1);
			}
		}
		else{
			if (strncmp(val.c, "cccc\0", 5) != 0){
				printf("cccc-unequal------%d-%s\n", i, val.c);
				exit(-1);
			}
		}
	}
	puts("check    ok---------------------------");

	return 0;
}