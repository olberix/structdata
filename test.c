#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define PRINTF_INT(X) printf("%d\n", (X));
#define PRINTF_INT64(X) printf("%x\n", (X));
#define PRINTF_FLOAT(X) printf("%f\n", (X));
#define PRINTF_STRING(X) printf("%s\n", (X));

typedef struct BNode{
	int a;
	double b;
}BNode;

void func()
{
	BNode node;
	printf("%f\n", node.b);
}

int main(int argc, char const *argv[])
{
	char str[256] = "abcdef";
	memmove(str + 3, str + 2, 4);
	PRINTF_STRING(str);
	return 0;
}