#include "DuCirLinkList.h"
#include <time.h>

typedef struct StC{
	double c;
	double b;
	int a;
}StC;

void display(size_t key, void* elem, void* args)
{
	StC* tmp = (StC*)elem;
	printf("[%lu]\t%d\t\t%lf\n", key, tmp->a, tmp->b);
}

void multi(size_t key, void* elem, void* args)
{
	StC* tmp = (StC*)elem;
	tmp->a *= 10;
	tmp->b *= 10;
}

bool cmp(const void* _lhs, const void* _rhs)
{
	StC* lhs = (StC*)_lhs;
	StC* rhs = (StC*)_rhs;
	if (lhs->a == rhs->a)
		return lhs->b < rhs->b;
	return lhs->a < rhs->a;
}

int main()
{
	DuCirLinkList* list = DucList().create(sizeof(StC));
	StC tmp = {10.1, 1.9, 3};
	DucList().push_back(list, &tmp);
	tmp.a = 20;
	tmp.b = 2.1;
	DucList().push_back(list, &tmp);
	DucList().reverse(list);
	tmp.a = 33;
	tmp.b = 31.256;
	DucList().insert(list, 0, &tmp);
	tmp.a = 47;
	tmp.b = 41.333;
	DucList().insert(list, 3, &tmp);
	DucList().erase(list, 2);
	DucList().reverse(list);
	DucList().r_for_each(list, multi, NULL);
	// DucList().for_each(list, display, NULL);

	DucList().clear(list);
	tmp.a = 588;
	tmp.b = 5.1;
	DucList().insert(list, 0, &tmp);
	// DucList().for_each(list, display, NULL);

	tmp.a = 23;
	tmp.b = 177.2;
	DucList().insert(list, 1, &tmp);
	tmp.a = 29;
	tmp.b = 1.02;
	DucList().insert(list, 1, &tmp);
	tmp.a = 2;
	tmp.b = 3.01;
	DucList().push_back(list, &tmp);
	DucList().reverse(list);
	srand(time(0));
	for (int i = 0; i < 150; i++){
		tmp.a = rand();
		tmp.b = rand() + 0.25;
		DucList().push_back(list, &tmp);
		DucList().reverse(list);
	}
	DucList().for_each(list, display, NULL);
	printf("========================================\n");
	// DucList().for_each(list, display, NULL);
	// DucList().erase(list, 0);
	// DucList().erase(list, 0);
	// DucList().erase(list, 0);

	// for (int i = 0; i < 1000000; i++){
	// 	tmp.a = i + 1;
	// 	tmp.b = 985544.0f/(i + 1);
	// 	DucList().push_back(list, &tmp);
	// }
	// printf("%d\n", DucList().length(list));
	// tmp = DucList().at(list, 729999);
	// printf("%d\t%lf\n", tmp.a, tmp.b);
	DucList().sort(list, cmp);
	// DucList().for_each(list, display, NULL);
	DucList().destroy(&list);

	list = DucList().create(sizeof(int));
	int a = 10;
	DucList().insert(list, 0, &a);
	a = 11;
	DucList().insert(list, 0, &a);
	a = 12;
	DucList().insert(list, 0, &a);
	a = 13;
	DucList().push_back(list, &a);
	a = 14;
	DucList().push_back(list, &a);
	a = 15;
	DucList().insert(list, 0, &a);
	DULIST_FOREACH(list, int, {
		printf("[%lu]\t%d\n", key, value);
	});
	DULIST_FOREACH_REVERSE(list, int, {
		printf("[%lu]\t%d\n", key, value);
	});
	a = 123;
	DucList().change(list, 0, &a);
	int d = TOCONSTANT(int, DucList().at(list, 0));
	printf("%d\n", d);

	DucList().swap(list, 0, 5);
	DucList().swap(list, 0, 5);
	DULIST_FOREACH(list, int, {
		printf("[%lu]\t%d\n", key, value);
	});

	return 0;
}