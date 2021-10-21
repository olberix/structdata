#include <string.h>
#include <stdio.h>
#include "SqStack/SqStack.h"
#include <stdnoreturn.h>
#define PRINTF_INT(X) printf("%d\n", (X));
#define PRINTF_INT64(X) printf("%x\n", (X));
#define PRINTF_FLOAT(X) printf("%f\n", (X));
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

 /*此行确保ptr指向类型和member类型一致,编译后可能优化掉*/
#define container_of(ptr, type, member) ({          \
    const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})


_Noreturn void test1()
{
	puts("111111-test1");
}

/*_Noreturn*/ void test2()
{
	puts("222222-test2");
	test1();
}

int main(int argc, char const *argv[])
{
	test2();
	puts("main-main");
	return 0;
}