#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

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
		return "dragonshell: memory allocation error\n";
	}
	
	// Get the current working directory and check if was successful or not
	if (getcwd(cwd, MAX_PATH) == NULL) {
		free(cwd);
		*success = 0;
		return "dragonshell: getcwd() error\n";
	}

	*success = 1;
	return strcat(cwd, "\n");
}

char *cd(char **commandArgs, int *success) {
	// Check if the user inputted a second argument for the new path, return if the didn't
	if (commandArgs[1] == NULL) {
		*success = 0;
		return "dragonshell: No directory given\n";
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
		return "dragonshell: Error when creating new path\n";
	}
	free(cwd);

	// Move into that directory and give an error if it was unsuccessful
	if (chdir(newPath) == -1) {
		*success = 0;
		return "dragonshell: No such file or directory\n";
	}
	
	*success = 1;
	return NULL;
}

char *runExternalProgram(char **commandArgs, int asBackground, char *outputFile, int *success) {
	pid_t pid = fork();

	if (pid < 0) {	
		*success = 0;
		return "dragonshell: fork() failed\n";
	}

	// Child process
	if (pid == 0) {
		if (outputFile != NULL) {
			int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 00644);
			if (fd == -1) {
				*success = 0;
				return "dragonshell: Error in opening file\n";
			}
			// Redirect stdout to the output file
			if (dup2(fd, STDOUT_FILENO) == -1) {
				close(fd);
				*success = 0;
				return "dragonshell: dup2() failed\n";
			}
			close(fd);

		}
		
		// Execute the external program
		if (execve(commandArgs[0], commandArgs, NULL) == -1) {
			_exit(1);
			*success = 0;
			return "dragonshell: Command not found\n";
		}

		/*if (write(fd, message, strlen(message)) == -1) {
			printf("dragonshell: Error writing to file\n");
			close(fd);
			return 1;
		}
	
		close(fd);
		return 0;
		}*/
	
	// Parent process
	} else if (pid > 0) {
		// Run the child process in the background
		if (asBackground == 0) {
			*success = 1;
			char buffer[256];
			sprintf(buffer, "dragonshell: running process in background with PID %d\n", pid);
			return strdup(buffer);
		}

		// If not running as background process, Wait for the child process to finish
		int status;
		if (waitpid(pid, &status, 0) == -1) {
			*success = 0;
			return "dragonshell: waitpid() failed\n";
		}
	}

	*success = 1;
	return NULL;


	/*
	// Fork the given process
	pid_t pid;
	pid = fork();

	if (pid == 0) {
		if (outputFile != NULL) {
			int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 00644);
			if (fd == -1) {
				_exit(1);	// Prevent zombie processes
				*success = 0;
				return "dragonshell: Failed to open output file\n";
			}
			// Redirect stdout to the output file
			if (dup2(fd, STDOUT_FILENO) == -1) {
				_exit(1);	
				close(fd);
				*success = 0;
				return "dragonshell: dup2() failed\n";
			}
			close(fd);
		}
		
		// Child process: run the external program
		if (execve(commandArgs[0], commandArgs, NULL) == -1) {
			_exit(1);
			*success = 0;
			return "dragonshell: Command not found\n";
		}
	} else if (pid > 0) {
		// Parent process		
		
	} else {
	}

	*success = 1;
	return NULL;
	*/
}

int outputToFile(char *message, char *file) {
	int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 00644);
	if (fd == -1) {
		printf("dragonshell: Error in opening file\n");
		return 1;
	}

	if (write(fd, message, strlen(message)) == -1) {
		printf("dragonshell: Error writing to file\n");
		close(fd);
		return 1;
	}
	
	close(fd);
	return 0;
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
		
		// Get the total number of command arguments (stops at NULL or >)
		int totalCommandArgs = 1;
		int outputToFileFlag = 0;
		while (commandArgs[totalCommandArgs] != NULL) {
			if (commandArgs[totalCommandArgs][0] == '>') {
				outputToFileFlag = 1;
				break;
			}
			totalCommandArgs++;
		}

		// Check if the command is outputted to a file
		char *outputFile = NULL;
		if (outputToFileFlag) {
			if (commandArgs[totalCommandArgs+1] == NULL) {
				printf("dragonshell: No ouptut file given\n");
				free(input);
				free(commandArgs);
				continue;
			}
			outputFile = commandArgs[totalCommandArgs+1];
		
			//Copy the data to a new array exluding the '>' and '<outputFile>' from the old commandArgs array
			char **tmp_commandArgs = (char **)malloc(totalCommandArgs); 
			for (int i = 0; i < totalCommandArgs; i++) {
				tmp_commandArgs[i] = commandArgs[i];
			}
			free(commandArgs);
			commandArgs = tmp_commandArgs;
		}


		// Command: pwd
		if (strcmp(commandArgs[0], "pwd") == 0) {
			int success;
			char *cwd = pwd(&success);
			if (outputFile == NULL) {
				printf("%s", cwd);
			} else {
				outputToFile(cwd, outputFile);
			}
			free(cwd);

		// Command: cd
		} else if (strcmp(commandArgs[0], "cd") == 0) {
			int success;
			char *outStr = cd(commandArgs, &success);
			if (!success) {
				if (outputFile == NULL) {
					printf("%s", outStr);
				} else {
					outputToFile(outStr, outputFile);
				}
			}

		// Command: exit
		} else if (strcmp(commandArgs[0], "exit") == 0) {
			inShell = 0;	// Exit the loop
		
		// If no command was inputted then run an external program
		} else {
			int success;
			int runInBackground = strcmp(commandArgs[totalCommandArgs-1], "&");
			char *outStr = runExternalProgram(commandArgs, runInBackground, outputFile, &success);
			/*if (!success) {
				if (outputFile == NULL) {
					printf("%s", outStr);
				} else {
					outputToFile(outStr, outputFile);
				}
			}*/
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
// ask if we need the -Wextra flag to be clear for full marks
//
//
// HANDLE SIGNALS: you need to handle ctrl+c and ctrl+z
//
//
// make background procesess only a total of 1
//
// do we have to check for both input and output ( ie.  <  and  >  )
