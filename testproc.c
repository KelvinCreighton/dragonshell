#include <stdio.h>

int main(int argc, char *argv[]) {
	
	if (argv[0] == NULL) {
		printf("testproc: noargs");
	} else {
		printf("testproc: %s\n", argv[0]);
	}
	
	return 0;
}
