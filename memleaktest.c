#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main() {
	char *s1 = (char *)malloc(10);
	char *s2 = (char *)malloc(10);
 strcpy(s1, "asdf");
    strcpy(s2, "cool");	
	printf("s1: %s\n", s1);
	printf("s2: %s\n", s2);

	free(s1);
	s1 = s2;
	
	printf("s1: %s\n", s1);
	free(s2);
	return 0;
}
