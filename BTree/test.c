#include "BTree.h"
#include <stdio.h>

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
	char c;
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
	printf("KEY:a=%d b=%lf\tVALUE:a=%d b=%lf c=%c\n", key->a, key->b, value->a, value->b, value->c);
}

int main(int argc, char const *argv[])
{
	BTree* bt = BTree().create(sizeof(TYKEY), sizeof(TYVALUE), equalFunc, lessFunc, "cc.DATA");
	TYKEY key;
	TYVALUE val;
	// for (int i = 0; i < 750000; i++){
	// 	key.a = i;
	// 	key.b = i + 0.5;
	// 	val.a = 3 * i + 1;
	// 	val.b = val.a + 0.123;
	// 	val.c = 'e';
	// 	BTree().insert(bt, &key, &val);
	// }


	// BTree().traverse(bt, foreach);
	// puts("=======================================================");
	// puts("=======================================================");
	// BTree().level_order_traverse(bt, foreach);

	for(int i = 0; i < 750000; i++){//user:0.648s sys:3.227s 这内存拷贝真的快
		key.a = i;
		key.b = key.a + 0.5;
		val = TOCONSTANT(TYVALUE, BTree().at(bt, &key));
		if (!(val.a == key.a * 3 + 1 && val.b == val.a + 0.123)){
			puts("error---------");
			break;
		}
		if (i % 10000 == 0)
			puts("true---------");
	}
	BTree().destroy(&bt);
	return 0;
}