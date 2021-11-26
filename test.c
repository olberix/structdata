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
int main(int argc, char const *argv[])
{
	int fd = open("dd.a", O_RDWR | O_CREAT, 777);
	char *str = "12345678";
	for (int i = 0; i < 4096 / 8; i++)
		write(fd, str, 8);
	for (int i = 0; i < 4096 / 8; i++)
		write(fd, str, 8);
	for (int i = 0; i < 4096 / 8; i++)
		write(fd, str, 8);
	for (int i = 0; i < 4096 / 8; i++)
		write(fd, str, 8);
	for (int i = 0; i < 4096 / 8; i++)
		write(fd, str, 8);

	printf("%d--------\n", lseek(fd, 4096 * 5 - 1, SEEK_HOLE));
	// fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, 4096, 4096);

	fsync(fd);
	close(fd);


	return 0;
}