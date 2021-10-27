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
	// for (size_t i = 0; i < 20; i++){
	// 	key.a = rand()%100;
	// 	key.b = rand()%100 + 0.25;
	// 	val.a = rand()%100;
	// 	val.b = rand()%100 + 0.25;
	// 	// printf("key:%d\t%f\tval:%d\t%f\n", key.a, key.b, val.a, val.b);
	// 	RBTree().insert(tree, &key, &val);
	// }
	// puts("=========================");
	// RBTree().level_order_traverse(tree, foreach);

	// puts("=========================");
	// key.a = 83;
	// key.b = 86.25;
	// RBTree().erase(tree, &key);
	// RBTree().level_order_traverse(tree, foreach);

	// int arry[] = {2,8,10,15,25,27,32,50,55,60,78,80,455,421,566,43,44,224,3,45,64,4,21,41,54,5};
	// int arry[] = {2,8,10,15,25,27};
		int arry[] = {326,600,625,687,384,958,526,948,420,203,24,808,770,165,618,343,348,136,85,402,781,289,
		598,829,539,197,951,935,975,704,364,271,202,801,37,242,80,796,773,576,706,968,334,175,21,78,577,
		599,494,877,75,601,510,559,401,258,664,459,826,541,137,690,959,219,279,668,229,993,71,270,728,146,
		661,836,512,743,933,267,838,769,614,185,774,853,498,864,284,919,595,979,607,798,841,285,680,551,811,870,383,58};


	for (size_t i = 0; i < sizeof(arry) / sizeof(int); i++){
		key.a = arry[i];
		RBTree().insert(tree, &key, &val);
	}
	// puts("=========================");
	// RBTree().level_order_traverse(tree, foreach);

	int ssss[] = {326,600,625,687,384,958,526,948,420,203,24,808,770,165,618,343,348,136,85,402,781,289,
		598,829,539,197,951,935,975,704,364,271,202,801,37,242,80,796,773,576,706,968,334,175,21,78,577,};
	// int ssss[] = {326,600,625,687,384,958,526,948,420,203,24,808,770,165,618,343,348,136,85,402,781,289,
	// 	598,829,539,197,951,935,975,704,364,271,202,801,37,242,80,796,773,576,706,968,334,175,21,78,577,
	// 	599,494,877,75,601,510,559,401,258,664,459,826,541,137,690,959,219,279,668,229,993,71,270,728,146,
	// 	661,836,512,743,933,267,838,769,614,185,774,853,498,864,284,919,595,979,607,798,};

	// int ssss[] = {2,8,10,15,25,27};
	for (size_t i = 0; i < sizeof(ssss) / sizeof(int); i++){
		key.a = ssss[i];
		RBTree().erase(tree, &key);
	}
	puts("=========================");
	RBTree().level_order_traverse(tree, foreach);
	// puts("=========================");
	// RBTree().traverse(tree, foreach);
	// puts("=========================");
	// RBTree().clear(tree);
	// RBTree().level_order_traverse(tree, foreach);

	RBTree().destroy(&tree);
	return 0;
}