#include "SkipList.h"

int main(int argc, char const *argv[])
{
	SkipList* list = SkipList().create(sizeof(int), default_equal_func_int32, default_less_func_int32);
	
	SkipList().destroy(&list);
	return 0;
}