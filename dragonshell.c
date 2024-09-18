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

char *pwd(int *success) {
	// Allocate space for the current working directory path
	char *cwd = malloc(MAX_PATH);
	// Check if the allocation had an error
	if (cwd == NULL) {
		*success = 0;
		return "dragonshell: memory allocation error";
	}
	
	// Get the current working directory and check if was successful or not
	if (getcwd(cwd, MAX_PATH) == NULL) {
		free(cwd);
		*success = 0;
		return "dragonshell: getcwd() error";
	}

	*success = 1;
	return cwd;
}

char *cd(char **commandArgs, int *success) {
	// Check if the user inputted a second argument for the new path, return if the didn't
	if (commandArgs[1] == NULL) {
		*success = 0;
		return "dragonshell: No directory given";
	}

	// Get the current working directory
	int pwdSuccess;
	char *cwd = pwd(&pwdSuccess);
	if (!pwdSuccess) {
		*success = 0;
		return cwd;	// MEMORY LEAK HERE????
	}
	
	// Combine the current working directory with the users second argument as one full newPath
	char newPath[MAX_PATH];
	char *separator = "/";
	snprintf(newPath, sizeof(newPath), "%s%s%s", cwd, separator, commandArgs[1]);

	if (newPath == NULL) {
		free(cwd);
		*success = 0;
		return "dragonshell: Error when creating new path";
	}
	free(cwd);

	// Move into that directory and give an error if it was unsuccessful
	if (chdir(newPath) == -1) {
		*success = 0;
		return "dragonshell: No such file or directory";
	}
	
	*success = 1;
	return NULL;
}

char *runExternalProgram(char **commandArgs, int asBackground, int *success) {
	// Fork the given process
	pid_t pid;
	pid = fork();

	if (pid == 0) {
		// Child process: run the external program
		if (execve(commandArgs[0], commandArgs, NULL) == -1) {
			_exit(1);	// Prevent zombie processes
			*success = 0;
			return "dragonshell: Command not found";
		}
	} else if (pid > 0) {
		// Parent process
		
		if (asBackground == 0) {	// Run the child process in the background
			printf("dragonshell: running process in background with PID %d\n", pid);
		} else {	// Wait for the child process to finish
			int status;
			waitpid(pid, &status, 0);
		}
	} else {
		// fork failed
		*success = 0;
		return "dragonshell: fork() failed";
	}

	*success = 1;
	return NULL;
}

int main(int argc, char *argv[]) {
	//welcomeBanner();
	
	int inShell = 1;
	while (inShell) {
		printf("dragonshell > ");
		
		// Read user command
		char *input = (char *)malloc(MAX_INPUT);
		fgets(input, MAX_INPUT, stdin);
		
		// Skip if the user didn't input anything
		if (input[1] == '\0') {
			free(input);
			continue;
		}
		// Remove the trailing '\n'
		input[strlen(input)-1] = '\0';
		
		// Split the command separated by spaces
		char **commandArgs = (char **)calloc(MAX_INPUT, sizeof(char *));
		tokenize(input, " ", commandArgs);
		// Get the total number of command arguments
		int totalCommandArgs = 1;
		while (commandArgs[totalCommandArgs] != NULL) {
			totalCommandArgs++;
		}

		// Command: pwd
		if (strcmp(commandArgs[0], "pwd") == 0) {
			int success;
			char *cwd = pwd(&success);	
			printf("%s\n", cwd);
			free(cwd);

		// Command: cd
		} else if (strcmp(commandArgs[0], "cd") == 0) {
			int success;
			char *outStr = cd(commandArgs, &success);
			if (!success) {
				printf("%s\n", outStr);
			}

		// Command: exit
		} else if (strcmp(commandArgs[0], "exit") == 0) {
			inShell = 0;	// Exit the loop
		
		// If no command was inputted then run an external program
		} else {
			int success;
			int runInBackground = strcmp(commandArgs[totalCommandArgs-1], "&");
			char *outStr = runExternalProgram(commandArgs, runInBackground, &success);
			if (!success) {
				printf("%s\n", outStr);
			}
		}

		
		free(input);
		free(commandArgs);
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
//
//
// HANDLE SIGNALS: you need to handle ctrl+c and ctrl+z

