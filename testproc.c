#include <stdio.h>

int main(int argc, char *argv[]) {
	
	if (argv[1] == NULL) {
		printf("testproc: %s\n", argv[0]);
	} else {
		printf("testproc: %s %s\n", argv[0], argv[1]);
	}
	
	return 0;
}
