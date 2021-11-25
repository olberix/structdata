#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
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
#define N = 40961
int main(int argc, char const *argv[])
{
	// int fd = open("dd.a", O_RDWR | O_CREAT, 777);
	// char *str = "12345678";
	// for (int i = 0; i < 4096 / 8; i++)
	// 	write(fd, str, 8);
	// for (int i = 0; i < 4096 / 8; i++)
	// 	write(fd, str, 8);
	// for (int i = 0; i < 4096 / 8; i++)
	// 	write(fd, str, 8);
	// for (int i = 0; i < 4096 / 8; i++)
	// 	write(fd, str, 8);
	// for (int i = 0; i < 4096 / 8; i++)
	// 	write(fd, str, 8);

	// lseek(fd, 4096 * 3, SEEK_SET);
	// fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 4096, 4096);

	// fsync(fd);
	// close(fd);


	int fd1 = open("BTree/cc.DATA", O_RDWR | O_CREAT, 777);
	int fd2 = open("BTree/111.DATA", O_RDWR | O_CREAT, 777);
	char str1[40961];
	char str2[40961];
	long long aa = 0;
	do{
		memset(str1, 0, 40961);
		memset(str2, 0, 40961);
		int ret1 = read(fd1, str1, 40960);
		int ret2 = read(fd2, str2, 40960);
		if (ret1 != ret2){
			puts("111111111");
			break;
		}
		if (strcmp(str1, str2) != 0){
			puts("222222222");
			break;
		}
		if (ret1 == 0)
			break;
		aa++;
		if (aa % 100 == 0)
			printf("%d 4k-----------\n", aa);
	}while(true);

	puts("ok---------------");

	close(fd1);
	close(fd2);

	return 0;
}