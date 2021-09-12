#include <string.h>
#include <stdio.h>
#include "SqStack/SqStack.h"
#define PRINTF_INT(X) printf("%d\n", (X));
#define PRINTF_INT64(X) printf("%x\n", (X));
#define PRINTF_FLOAT(X) printf("%f\n", (X));
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

 /*此行确保ptr指向类型和member类型一致,编译后可能优化掉*/
#define container_of(ptr, type, member) ({          \
    const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})


int main(int argc, char const *argv[])
{
	int a[] = {1, 2, 3};
	PRINTF_INT(sizeof(a) / sizeof(int));
	return 0;
}