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
	// printf("KEY:a=%d b=%lf\tVALUE:a=%d b=%lf\n", key->a, key->b, value->a, value->b);
}

int main(int argc, char const *argv[])
{
	BPTree* tree = BPTree().create(sizeof(TYKEY), sizeof(TYVALUE), equalFunc, lessFunc, "../../bp");


	TYKEY key;
	TYVALUE val;
	memcpy(val.c, "abcdefghijklmnopqrstuvwxyz\0", 27);
	// memcpy(key.c, "abcdefghijklmnopqrstuvwxyz\0", 27);
	int range = 250000;
	for(int i = 0; i < range; i++){
		key.a = i;
		key.b = key.a + 0.5;
		val.a = key.a * 2;
		val.b = val.a + 0.5;
		BPTree().insert(tree, &key, &val);
		printf("%d-----done\n", i);
	}

	// BPTree().traverse(tree, foreach);
	puts("===================");
	for(int i = 0; i < range; i++){
		key.a = i;
		key.b = key.a + 0.5;
		const void* ret = BPTree().at(tree, &key);
		val = TOCONSTANT(TYVALUE, ret);
		if (!(val.a == key.a * 2 && val.b == val.a + 0.5)){
			printf("%d--error---------", key.a);
			break;
		}
		if (i % 30000 == 0)
			puts("true---------");
	}
	puts("===================");
	BPTree().destroy(&tree);
	return 0;
}