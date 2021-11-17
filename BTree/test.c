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
	TYKEY key = {12, 0.32323277};
	TYVALUE val = {15, 17.8116, 'a'};
	BTree().insert(bt, &key, &val);
	BTree().traverse(bt, foreach);
	// BTree().level_order_traverse(bt, foreach);
	BTree().destroy(&bt);
	return 0;
}