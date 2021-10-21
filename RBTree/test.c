#include "RBTree.h"
#include <time.h>

typedef struct KEY
{
	int a;
	float b;
}KEY;

typedef struct VALUE
{
	int a;
	float b;
	char c;
}VALUE;

bool key_equal(const void* lhs, const void* rhs)
{
	const KEY* _lhs = lhs;
	const KEY* _rhs = rhs;
	return _lhs->a == _rhs->a && _lhs->b == _rhs->b;
}

bool key_less(const void* lhs, const void* rhs)
{
	const KEY* _lhs = lhs;
	const KEY* _rhs = rhs;
	if (_lhs->a == _rhs->a)
		return _rhs->b < _lhs->b;
	return _rhs->a < _lhs->a;
}

void foreach(const void* key, void* val)
{
	const KEY* _key = key;
	const VALUE* _val = val;
	printf("key:%d\t%f\tval:%d\t%f\n", _key->a, _key->b, _val->a, _val->b);
}

int main()
{
	RBTree* tree = RBTree().create(sizeof(KEY), sizeof(VALUE), key_equal, key_less);
	KEY key = {1,1.1};
	VALUE val = {10, 1.1, 'c'};
	for (size_t i = 0; i < 20; i++){
		key.a = rand()%100;
		key.b = rand()%100 + 0.25;
		val.a = rand()%100;
		val.b = rand()%100 + 0.25;
		printf("key:%d\t%f\tval:%d\t%f\n", key.a, key.b, val.a, val.b);
		RBTree().insert(tree, &key, &val);
	}
	puts("=========================");
	RBTree().level_order_traverse(tree, foreach);
	puts("=========================");
	RBTree().traverse(tree, foreach);
	puts("=========================");
	RBTree().clear(tree);
	RBTree().level_order_traverse(tree, foreach);

	RBTree().destroy(&tree);
	return 0;
}