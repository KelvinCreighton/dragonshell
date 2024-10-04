#include <stdio.h>

int main(int argc, char *argv[]) {
	int num = argv[1][0]-48;
	if (argv[1][1] != '\0') {
		num *= 10;
		num += argv[1][1]-48;
}	
	while (num > 0) {
	long count = 800000000;
	while (count > 0) {
		count--;
	}
	printf("num: %d\n", num);
	num--;
}
	printf("end of testproc\n");
	return 0;
}
