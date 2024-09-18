#include <stdio.h>

int main() {
	
	for (int i = 0; i < 5; i++ ) {
		int nums[10];
		if (i < 2) {
			nums[i] = i+1;
		} else {
			printf("%d %d %d\n", nums[0], nums[1], nums[2]);
		}
	}

	

	return 0;
}
