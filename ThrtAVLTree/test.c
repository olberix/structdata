#include "ThrtAVLTree.h"
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

void foreach(const void* key, void* val, void* args)
{
	const KEY* _key = key;
	const VALUE* _val = val;
	printf("key:%d\t%f\tval:%d\t%f\n", _key->a, _key->b, _val->a, _val->b);
}

int main()
{
	AVLTree* tree = AVLTree().create(sizeof(KEY), sizeof(VALUE), key_equal, key_less);
	KEY key;
	VALUE val;
	for (size_t i = 0; i < 7; i++){
		key.a = rand()%100;
		key.b = rand()%100 + 0.25;
		val.a = rand()%100;
		val.b = rand()%100 + 0.25;
		// printf("key:%d\t%f\tval:%d\t%f\n", key.a, key.b, val.a, val.b);
		AVLTree().insert(tree, &key, &val);
		if(key.a%2 == 0 || key.a%3==0 || key.a %5==0 || key.a%7==0)
			AVLTree().erase(tree, &key);
	}

	// AVLTree().clear(tree);
	int arry[] = {2,8,10,15,25,27,32,50,55,60,78,80,455,421,566,43,44,224,3,45,64,4,21,41,54,5};
	for (size_t i = 0; i < sizeof(arry) / sizeof(int); i++){
		key.a = arry[i];
		val.a = arry[i];
		AVLTree().insert(tree, &key, &val);
	}
	printf("tree size:%lu------\n", AVLTree().size(tree));

	for (size_t i = sizeof(arry) / sizeof(int) - 1; i >=5; i--){
		key.a = arry[i];
		AVLTree().erase(tree, &key);
	}
	printf("tree size:%lu------\n", AVLTree().size(tree));

	key.a = 10;
	val.a = 10555;
	AVLTree().change(tree, &key, &val);
	const void* p = AVLTree().at(tree, &key);
	printf("%d\n", ((VALUE*)p)->a);
	// AVLTree().erase(tree, &key);
	// printf("%p\n", &tree->thrtHead);
	// printf("%p\n", tree->root);
	// printf("%p\n", tree->root->lchild);
	// printf("%p\n", tree->root->rchild);
	// key.a = 20;
	// AVLTree().insert(tree, &key, &val);
	// key.a = 10;
	// AVLTree().insert(tree, &key, &val);
	// printf("%d\n", tree->root->height);
	// key.a = 5;
	// AVLTree().insert(tree, &key, &val);
	// key.a = 4;
	// AVLTree().insert(tree, &key, &val);
	// key.a = 3;
	// AVLTree().insert(tree, &key, &val);
	// key.a = 5;
	// val.a = 288;
	// AVLTree().insert(tree, &key, &val);
	// printf("%s\n", "===============================");
	// AVLTree().pre_order_traverse(tree, foreach, NULL);
	// AVLTree().pre_order_traverse_st(tree, foreach, NULL);
	// printf("%s\n", "===============================");
	// AVLTree().in_order_traverse(tree, foreach, NULL);
	// AVLTree().in_order_traverse_st(tree, foreach, NULL);
	// printf("%s\n", "===============================");
	// AVLTree().post_order_traverse(tree, foreach, NULL);
	// AVLTree().post_order_traverse_st(tree, foreach, NULL);
	printf("%s\n", "===============================");
	AVLTree().level_order_traverse(tree, foreach, NULL);
	printf("%s\n", "===============================");
	AVLTree().in_order_traverse_thrt(tree, foreach, NULL);
	printf("tree size:%lu------\n", AVLTree().size(tree));
	AVLTree().destroy(&tree);
	return 0;
}