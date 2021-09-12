#include "SqList.h"
#include <time.h>

typedef struct StA{
	int a;
	double b;
}StA;

void display(size_t key, void* elem)
{
	StA* tmp = (StA*)elem;
	printf("[%d]\t%d\t%lf\n", key, tmp->a, tmp->b);
}

void multi(size_t key, void* elem)
{
	StA* tmp = (StA*)elem;
	tmp->a *= 10;
	tmp->b *= 10;
}

bool cmp(const void* _lhs, const void* _rhs)
{
	StA* lhs = (StA*)_lhs;
	StA* rhs = (StA*)_rhs;
	if (lhs->a == rhs->a)
		return lhs->b < rhs->b;
	return lhs->a < rhs->a;
}

int main()
{
	SqList* list = SqList().create(sizeof(StA), NULL);
	StA tmp = {10, 220.3};
	SqList().push_back(list, &tmp);
	SqList().r_for_each(list, &multi);
	SqList().erase(list, 0);
	srand(time(0));
	for (int i = 0; i < 50; i++){
		tmp.a = i;
		tmp.a = rand();
		tmp.b = tmp.a + 0.1;
		SqList().push_back(list, &tmp);
		SqList().reverse(list);
	}
	// SqList().for_each(list, &display);
	SqList().reverse(list);
	puts("++++++++++++++++++++++++++++++++");
	// SqList().for_each(list, &display);
	puts("++++++++++++++++++++++++++++++++");
	SqList().sort(list, &cmp);
	// SqList().for_each(list, &display);
	SQLIST_FOREACH(list, StA, {
		if (_PVALUE__->a % 2 == 0){
			SqList().erase(list, _INDEX__);
			_INDEX__--;
			_PVALUE__--;
		}
	})
	puts("++++++++++++++++++++++++++++++++");
	SQLIST_FOREACH(list, StA, {
		printf("[%d]\t%d\t%lf\n", _INDEX__, _PVALUE__->a, _PVALUE__->b);
	})
	tmp.a = 123456789;
	SqList().change(list, 0, &tmp);
	puts("++++++++++++++++++++++++++++++++");
	SQLIST_FOREACH(list, StA, {
		printf("[%d]\t%d\t%lf\n", _INDEX__, _PVALUE__->a, _PVALUE__->b);
	})
	// puts("++++++++++++++++++++++++++++++++");
	// SQLIST_FOREACH_REVERSE(list, StA, {
	// 	printf("[%d]\t%d\t%lf\n", _INDEX__, _PVALUE__->a, _PVALUE__->b);
	// })


	// const StA* res = SqList().at(list, 0);
	// printf("%d\n", res->a);
	// for (int i = 0; i < 49; i++){
	// 	SqList().erase(list, 0);
	// }
	// res = SqList().at(list, 0);
	// printf("%d\n", res->a);
	// printf("%d\n", SqList().length(list));
	// SqList().clear(list);
	// SqList().insert(list, 0, res);
	// StA sec = TOCONSTANT(StA, SqList().at(list, 0));
	// printf("%d\n", sec.a);
	SqList().destroy(&list);
	return 0;
}
