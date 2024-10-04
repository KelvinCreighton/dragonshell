#include <stdio.h>

int main() {
for (int i = 0; i < 100000; i++) {
    FILE *file = fopen("testfile.txt", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
    
        fprintf(file, "%d", i);

    fclose(file);
}
    return 0;
}
