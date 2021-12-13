#include "BTree.h"
#include <stdio.h>
#include <string.h>

#define PRINTF_INT(X) printf("%d\n", (X));
#define PRINTF_INT64(X) printf("%x\n", (X));
#define PRINTF_FLOAT(X) printf("%f\n", (X));
#define PRINTF_STRING(X) printf("%s\n", (X));

typedef struct TYKEY{
	int a;
	double b;
}TYKEY;

typedef struct TYVALUE{
	int a;
	double b;
	char c[1024];
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
	printf("KEY:a=%d b=%lf\tVALUE:a=%d b=%lf c=%s\n", key->a, key->b, value->a, value->b, value->c);
}
extern int cccc;

int main(int argc, char const *argv[])
{
	BTree* bt = BTree().create(sizeof(TYKEY), sizeof(TYVALUE), equalFunc, lessFunc, "../../cc.DATA");
	TYKEY key;
	TYVALUE val;
	memcpy(val.c, "abcdefghijklmnopqrstuvwxyz\0", 27);
	// val.c = 'e';
	const int range = 750000;
	puts("111111-----**********");
	for (int i = 0; i < range; i += 3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BTree().insert(bt, &key, &val);
		printf("11111----insert-%d\n", i);
	}
	puts("222222-----**********");
	for (int i = range - 1; i >= 0; i -= 3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BTree().insert(bt, &key, &val);
		printf("22222----insert-%d\n", i);
	}
	puts("333333-----**********");
	for (int i = range - 2; i >= 0; i -= 3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BTree().insert(bt, &key, &val);
		printf("33333----insert-%d\n", i);
	}

	puts("444444-----**********");
	for (int i = 0; i < range; i=i+2){
		key.a = i;
		key.b = key.a + 0.5;
		BTree().erase(bt, &key);
		printf("done-erase--2--%d\n", i);
	}
	puts("555555-----**********");
	for (int i = 0; i < range; i=i+2){
		key.a = i;
		key.b = key.a + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BTree().insert(bt, &key, &val);
		printf("done-insert--2--%d\n", i);
	}

	for (int i = 0; i < range / 2; i=i+3){
		key.a = i;
		key.b = key.a + 0.5;
		BTree().erase(bt, &key);
		printf("done-erase--3--%d\n", i);
	}

	for (int i = range - 3; i >= range / 2; i=i-3){
		key.a = i;
		key.b = key.a + 0.5;
		BTree().erase(bt, &key);
		printf("done-erase--3--%d\n", i);
	}

	for (int i = 0; i < range; i=i+3){
		key.a = i;
		key.b = i + 0.5;
		val.a = 3 * i + 1;
		val.b = val.a + 0.123;
		BTree().insert(bt, &key, &val);
		printf("done-insert--3--%d\n", i);
	}

	// for(int i = range - 1; i >= 0; i--){
	// 	key.a = i;
	// 	key.b = i + 0.5;
	// 	val.a = 3 * i + 1;
	// 	val.b = val.a + 0.123;
	// 	BTree().insert(bt, &key, &val);
	// }


	// BTree().traverse(bt, foreach);
	// puts("=======================================================");
	// puts("=======================================================");
	// BTree().level_order_traverse(bt, foreach);



	puts("=======================================================");
	for(int i = 0; i < range; i += 1){
		key.a = i;
		key.b = key.a + 0.5;
		const void* ret = BTree().at(bt, &key);
		if (ret){
			val = TOCONSTANT(TYVALUE, ret);
			if (!(val.a == key.a * 3 + 1 && val.b == val.a + 0.123)){
				puts("error---------");
				break;
			}
			if (i % 30000 == 0)
				puts("true---------");
		}
		else
		{
			PRINTF_INT(key.a);
		}
	}


	printf("hole-------%ld\n", lseek(bt->fd, 4096, SEEK_HOLE));
	BTree().destroy(&bt);
	return 0;
}