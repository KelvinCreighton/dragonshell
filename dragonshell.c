#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_INPUT 1024
#define MAX_PATH 1024

/**
 * @brief Tokenize a C string 
 * 
 * @param str - The C string to tokenize 
 * @param delim - The C string containing delimiter character(s) 
 * @param argv - A char* array that will contain the tokenized strings
 * Make sure that you allocate enough space for the array.
 */
void tokenize(char* str, const char* delim, char ** argv){
  char* token;
  token = strtok(str, delim);
  for(size_t i = 0; token != NULL; ++i){
    argv[i] = token;
  token = strtok(NULL, delim);
  }
}

/*void welcomeBanner() {
	const char *welcomeArt = "\n"
"
"                                           .___,                                                               \n"
" __      __     _                        _          ___                                                    \n"
" \\ \\    / /___ | | __  ___  _ __   ___  | |_  ___  |   \\  _ _  __ _  __ _  ___  _ _                     \n"
"  \\ \\/\\/ // -_)| |/ _|/ _ \\| '  \\ / -_) |  _|/ _ \\ | |) || '_|/ _` |/ _` |/ _ \\| ' \\               \n"
"   \\_/\\_/ \\___||_|\\__|\\___/|_|_|_|\\___|  \\__|\\___/ |___/ |_|  \\__,_|\\__, |\\___/|_||_|           \n"
"                                                                    |___/                                  \n";

	printf("%s", welcomeArt);
}*/

int main(int argc, char *argv[]) {
	//welcomeBanner();

	// Read the users command
	char input[MAX_INPUT];
	do {
		printf("dragonshell > ");
		fgets(input, sizeof(input), stdin);
		// Ask again if no command was inputted
		if (input[1] == '\0')
			continue;
		// Remove the trailing '\n'
		for (int i = 2; i < sizeof(input); i++) {
			if (input[i] == '\0') {
                        	input[i-1] = '\0';
				break;
			}
		}
		
		// Split the command separated by spaces
		char *commandArgs[MAX_INPUT];
		tokenize(input, " ", commandArgs);

		// Command: pwd
		if (strcmp(commandArgs[0], "pwd") == 0) {
			char cwd[MAX_PATH];
			if (getcwd(cwd, sizeof(cwd)) != NULL) {
				printf("%s\n", cwd);
			} else {
				printf("dragonshell: getcwd() error\n");
			}
		
		// Command: cd
		} else if (strcmp(commandArgs[0], "cd") == 0) {
			// ADD CHECK HERE FOR NO GIVEN DIRECTORY

			char cwd[MAX_PATH];
			if (getcwd(cwd, sizeof(cwd)) == NULL) {
				printf("dragonshell: getcwd() error\n");
				continue;
			}

			char newPath[MAX_PATH];
			char *separator = "/";
			snprintf(newPath, sizeof(newPath), "%s%s%s", cwd, separator, commandArgs[1]);
			if (chdir(newPath) == -1) {
				printf("dragonshell: No such file or directory\n");
				printf("%s\n", strcat(cwd, commandArgs[1]));
			}
		} else {
			printf("Invalid command\n");
		}
	} while (strcmp(input, "exit") != 0);
	
	return 0;
}

















