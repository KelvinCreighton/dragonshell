#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

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

char *pwd() {
	// Allocate space for the current working directory path
	char *cwd = malloc(MAX_PATH);
	// Check if the allocation had an error
	if (cwd == NULL) {
		printf("dragonshell: memory allocation error\n");
		return NULL;
	}
	
	// Get the current working directory and check if was successful or not
	if (getcwd(cwd, MAX_PATH) == NULL) {
		printf("dragonshell: getcwd() error\n");
		free(cwd);
		return NULL;
	}
	return cwd;
}

int cd(char **commandArgs) {
	// Check if the user inputted a second argument for the new path, return if the didn't
	if (commandArgs[1] == NULL) {
		printf("dragonshell: No directory given");
		return 1;
	}

	// Get the current working directory
	char *cwd = pwd();
	if (cwd == NULL)
		return 1;
	
	// Combine the current working directory with the users second argument as one full newPath
	char newPath[MAX_PATH];
	char *separator = "/";
	snprintf(newPath, sizeof(newPath), "%s%s%s", cwd, separator, commandArgs[1]);

	// Move into that directory and give an error if it was unsuccessful
	if (chdir(newPath) == -1) {
		printf("dragonshell: No such file or directory\n");
		return 1;
	}
	
	return 0;
}

int runExternalProgram(char **commandArgs) {
	// Fork the given process
	pid_t pid;
	pid = fork();

	if (pid < 0) {
		// fork failed
		printf("dragonshell: fork() failed\n");
		return 1;
	} else if (pid == 0) {
		// Child process: run the external program
		char *envp[] = {NULL};
		if (execve(commandArgs[0], commandArgs, envp) == -1) {
			printf("dragonshell: exeve() failed\n");
			return 1;
		}
		printf("child process\n");
	} else {
		// Parent process: wait for the child to finish
		int status;
		waitpid(pid, &status, 0);
		printf("parent process, child process finished\n");
	}

	return 0;
}

int main(int argc, char *argv[]) {
	//welcomeBanner();

	// Read the users command
	int inShell = 1;
	char input[MAX_INPUT];
	while (inShell) {
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
			char *cwd = pwd();	
			if (cwd != NULL) {
				printf("%s\n", cwd);
				free(cwd);
			}

		// Command: cd
		} else if (strcmp(commandArgs[0], "cd") == 0) {
			cd(commandArgs);

		// Command: exit
		} else if(strcmp(commandArgs[0], "exit") == 0) {
			inShell = 0;	// Exit the loop
		
		// If no command was inputted then run an external program
		} else {
			runExternalProgram(commandArgs);
		}
	}
	
	return 0;
}
















// QUESTIONS FOR LAB
//
// when parent process is waiting for child process does it have to check how it exited (normally vs by signal)
//
// when passing arguments to the child process do i start from the name of the process itself or skip that (ie: a.out, arg1, arg2,... or skip it as just arg1, arg2,...
//
//
// ask if we need the -Wextra flag to be clear for full marks
