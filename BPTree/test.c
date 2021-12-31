#include "BPTree.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

typedef struct TYKEY{
	int a;
	double b;
	// char c[1024];
}TYKEY;

typedef struct TYVALUE{
	int a;
	double b;
	// char c[1024];
	// char c;
}TYVALUE;

bool equalFunc(const void* lh, const void* rh)
{
	const TYKEY* _lh = lh;
	const TYKEY* _rh = rh;
	return _lh->a == _rh->a && _lh->b == _rh->b;
}

bool lessFunc(const void* lh, const void* rh)
{
	const TYKEY* _lh = lh;
	const TYKEY* _rh = rh;
	if (_lh->a == _rh->a)
		return _rh->b < _lh->b;
	return _rh->a < _lh->a;
}

void foreach(const void* pKey, const void* pValue)
{
	const TYKEY* key = pKey;
	const TYVALUE* value = pValue;
	// printf("KEY:a=%d b=%lf\tVALUE:a=%d b=%lf c=%s\n", key->a, key->b, value->a, value->b, value->c);
	printf("KEY:a=%d b=%lf\tVALUE:a=%d b=%lf\n", key->a, key->b, value->a, value->b);
}

int main(int argc, char const *argv[])
{
	// BPTree* tree = BPTree().create(sizeof(TYKEY), sizeof(TYVALUE), equalFunc, lessFunc, "../../tmpNormal/bp");
	BPTree* tree = BPTree().create(sizeof(TYKEY), sizeof(TYVALUE), equalFunc, lessFunc, "../../bp");


	TYKEY key;
	TYVALUE val;
	// memcpy(val.c, "abcdefghijklmnopqrstuvwxyz\0", 27);
	// memcpy(key.c, "abcdefghijklmnopqrstuvwxyz\0", 27);
	int range = 30;
	// key.a = 100000;
	// key.b = 100000.5;
	// BPTree().at(tree, &key);
	for (int i = 0; i < range; i += 3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BPTree().insert(tree, &key, &val);
		printf("11111----insert-%d\n", i);
	}
	puts("222222-----**********");
	for (int i = range - 1; i >= 0; i -= 3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BPTree().insert(tree, &key, &val);
		printf("22222----insert-%d\n", i);
	}
	puts("333333-----**********");
	for (int i = range - 2; i >= 0; i -= 3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BPTree().insert(tree, &key, &val);
		printf("33333----insert-%d\n", i);
	}

	puts("444444-----**********");
	for (int i = 0; i < range; i=i+2){
		key.a = i;
		key.b = key.a + 0.5;
		BPTree().erase(tree, &key);
		printf("done-erase--2--%d\n", i);
	}
	puts("555555-----**********");
	for (int i = 0; i < range; i=i+2){
		key.a = i;
		key.b = key.a + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BPTree().insert(tree, &key, &val);
		printf("done-insert--2--%d\n", i);
	}

	for (int i = 0; i < range / 2; i=i+3){
		key.a = i;
		key.b = key.a + 0.5;
		BPTree().erase(tree, &key);
		printf("done-erase--3--%d\n", i);
	}

	for (int i = range - 3; i >= range / 2; i=i-3){
		key.a = i;
		key.b = key.a + 0.5;
		BPTree().erase(tree, &key);
		printf("done-erase--3--%d\n", i);
	}

	for (int i = 0; i < range; i=i+3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BPTree().insert(tree, &key, &val);
		printf("done-insert--3--%d\n", i);
	}

	// // BPTree().traverse(tree, foreach);
	tree = BPTree().rebuild(&tree);
	// tree = BPTree().rebuild(&tree);
	// tree = BPTree().rebuild(&tree);

	puts("===================");
	for(int i = 0; i < range; i++){
		key.a = i;
		key.b = key.a + 0.5;
		const void* ret = BPTree().at(tree, &key);
		if (!ret){
			printf("%d-----not find\n", key.a);
		}
		else{
			val = TOCONSTANT(TYVALUE, ret);
			if (!(val.a == key.a * 3 + 1 && val.b == val.a + 0.123)){
				printf("%d--error---------", key.a);
				break;
			}
			if (i % 30000 == 0)
				puts("true---------");
		}
	}
	puts("===================");
	BPTree().destroy(&tree);
	return 0;
}