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
	// 	val.a = 2 * i;
	// 	val.b = val.a + 0.5;
	// 	val.c = 'e';
	// 	BTree().insert(bt, &key, &val);
	// }
	// BTree().traverse(bt, foreach);
	// puts("=======================================================");
	// puts("=======================================================");
	// BTree().level_order_traverse(bt, foreach);
	int a[] = {0, 1, 100, 500, 2000, 10000, 15000, 30000, 50000, \
		80000, 120000, 250000, 370000, 370010, 480000, 480001, 590000, 700000,700001, 749998, 749995};
	for (unsigned int i = 0; i < sizeof(a) / sizeof(int); i++){
		key.a = a[i];
		key.b = key.a + 0.5;
		val = TOCONSTANT(TYVALUE, BTree().at(bt, &key));
		if (!(val.a == key.a * 2 && val.b == val.a + 0.5)){
			puts("error---------");
			break;
		}
		puts("true---------");
	}
	BTree().destroy(&bt);
	return 0;
}