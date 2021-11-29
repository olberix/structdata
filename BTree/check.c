#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

int main(int argc, char const *argv[])
{
	int fd1 = open("222.DATA", O_RDWR | O_CREAT, 777);
	// int fd2 = open("111.DATA", O_RDWR | O_CREAT, 777);
	// char str1[4097];
	// char str2[4097];
	// long long aa = 0;
	// do{
	// 	memset(str1, 0, 4097);
	// 	memset(str2, 0, 4097);
	// 	int ret1 = read(fd1, str1, 4096);
	// 	int ret2 = read(fd2, str2, 4096);
	// 	if (ret1 != ret2){
	// 		puts("111111111");
	// 		break;
	// 	}
	// 	if (strcmp(str1, str2) != 0){
	// 		printf("222222222---%d", lseek(fd1, 0, SEEK_CUR));
	// 		break;
	// 	}
	// 	if (ret1 == 0){
	// 		puts("333333333");
	// 		break;
	// 	}
	// 	aa++;
	// 	if (aa % 10000 == 0)
	// 		printf("%d 4k-----------\n", aa);
	// }while(true);


	off_t hole;
	off_t data = 0;
	off_t end = lseek(fd1, 0, SEEK_END);
	lseek(fd1, 0, SEEK_SET);
	while(true){
		hole = lseek(fd1, data, SEEK_HOLE);
		if (hole == -1 || hole == end){
			puts("end-----------");
			break;
		}
		data = lseek(fd1, hole, SEEK_DATA);
		off_t diff = data - hole;
		if (diff % 4096 != 0){
			printf("false----hole:%ld, data:%ld diff:%ld\n", hole, data, diff);
			break;
		}
		else
			printf("true----hole:%ld, data:%ld diff:%ld\n", hole, data, diff);
	}

	puts("ok---------------");

	close(fd1);
	// close(fd2);

	return 0;
}