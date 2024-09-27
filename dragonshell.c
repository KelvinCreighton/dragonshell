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

char *cd(char **commandArgs) {
	// Check if the user inputted a second argument for the new path, return if the didn't
	if (commandArgs[1] == NULL) {
		return "dragonshell: No directory given\n";
	}

	// Get the current working directory
	char cwd[MAX_PATH];
	getcwd(cwd, MAX_PATH);	
	// Combine the current working directory with the users second argument as one full newPath
	char newPath[MAX_PATH];
	char *separator = "/";
	snprintf(newPath, sizeof(newPath), "%s%s%s", cwd, separator, commandArgs[1]);	
	
	// Move into that directory and return an error if it was unsuccessful
	if (chdir(newPath) == -1) {
		return "dragonshell: No such file or directory\n";
	}
	// Return NULL if there was no issue
	return NULL;
}

char *runExternalProgram(char **commandArgs, char *inputFile, char *outputFile, int pipefd[2]) {
	pid_t pid = fork();

	// No process
	if (pid < 0) {
		return "dragonshell: fork() failed\n";
	
	
	// Child process
	} else if (pid == 0) {
		// Input from file to command
		if (inputFile != NULL) {
			int fd = open(inputFile, O_RDONLY);
			if (fd == -1) {
				_exit(1);
				return "dragonshell: Error in opening file\n";
			}
			// Redirect fd from the input file to stdin
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		
		// Command outputs to output file
		if (outputFile != NULL) {
			int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 00644);
			if (fd == -1) {
				_exit(1);
				return "dragonshell: Error in opening file\n";
			}
			// Redirect stdout to fd of the output file
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		
		// If the read end is not -1 then the command is requested to pipe
		if (pipefd[0] != -1) {
			// If the write end is also not -1 then direct stdout to the write end
			if (pipefd[1] != -1) {
				dup2(pipefd[1], STDOUT_FILENO);
			// If the write end is -1 then direct the read end to stdin
			} else {
				dup2(pipefd[0], STDIN_FILENO);
			}
		}
		/*int i = 0;
		while (commandArgs[i] != NULL) {
			printf("%s\n", commandArgs[i]);
			i++;
		}*/
		//printf("asdfasfd11111111111");
		// Execute the external program
		if (execve(commandArgs[0], commandArgs, NULL) == -1) {
			//printf("asdfasdf");
			_exit(1);
			return "dragonshell: Command not found\n";
		}
	

	// Parent process
	} else if (pid > 0) {
		// Run the child process in the background
		/*if (asBackground == 0) {
			*success = 1;
			char buffer[256];
			sprintf(buffer, "dragonshell: running process in background with PID %d\n", pid);
			return strdup(buffer);
		}*/

		// If not running as background process, Wait for the child process to finish
		int status;
		waitpid(pid, &status, 0);
	}

	return NULL;
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
		int totalCommandArgs = 0;	// The total arguments for a command before | or < or >
		int outputFileIndex = 0;	// The index of the tokenized array where the > symbol is (0 if it doesnt exist)
		int inputFileIndex = 0;		// Same as the outputFileIndex but for the < symbol
		int pipeIndex = 0;		// Same as the outputFileIndex but for the | symbol to pipe to another command
		int totalPipedCommandArgs = 0;	// Calculate the total arguments that occur after the | symbol
		int i = 0;
		while (commandArgs[i] != NULL) {
			if (pipeIndex > 0)
				totalPipedCommandArgs++;

			if (outputFileIndex == 0 && commandArgs[i][0] == '>') {
				outputFileIndex = i+1;
				pipeIndex = -1;		// If outputToFile exists then piping cannot exist so force it to -1 to stop it from searching for the | symbol
			}
			if (pipeIndex == 0 && commandArgs[i][0] == '|') {
				pipeIndex = i+1;
				outputFileIndex = -1;
				// The input file cannot be after the pipe so set this to -1 to stop searching for the < symbol
				if (inputFileIndex == 0)
					inputFileIndex = -1;
			}
			if (inputFileIndex == 0 && commandArgs[i][0] == '<')
				inputFileIndex = i+1;
			
			if (outputFileIndex == 0 && inputFileIndex == 0 && pipeIndex == 0)
				totalCommandArgs++;
			i++;
		}

		// Check if the command is outputted to a file
		char *outputFile = NULL;
		if (outputFileIndex > 0) {
			if (commandArgs[outputFileIndex] == NULL) {
				printf("dragonshell: No output file given\n");
				free(input);
				free(commandArgs);
				continue;
			}
			outputFile = commandArgs[outputFileIndex];
		}
		
		// Check if the command is piped to another command
		char **pipedCommandArgs;
		if (pipeIndex > 0) {
			if (totalPipedCommandArgs == 0) {
				printf("dragonshell: No pipe command given\n");
				free(input);
				free(commandArgs);
				continue;
			}
			// Copy the remaining command args after the pipe
			pipedCommandArgs = (char **)malloc( (totalPipedCommandArgs+1) * sizeof(char *));
			for (int i = 0; i < totalPipedCommandArgs; i++) {
				pipedCommandArgs[i] = commandArgs[pipeIndex+i];
			}
			pipedCommandArgs[totalPipedCommandArgs] = NULL;	// Set NULL terminator at the end of the argumnets
		}
		
		// Check if the command is taking input from a file
		char *inputFile = NULL;
		if (inputFileIndex > 0) {
			if (commandArgs[inputFileIndex] == NULL) {
				printf("dragonshell: No input file given\n");
				free(input);
				free(commandArgs);
				continue;
			}
			inputFile = commandArgs[inputFileIndex];
		}

		//Copy the data to a new array exluding the '>', '<', and '|' from the old commandArgs array (and anything after them)
		if (outputFileIndex > 0 || inputFileIndex > 0 || pipeIndex > 0) {
			char **tmp_commandArgs = (char **)malloc( (totalCommandArgs+1) * sizeof(char *));
			for (int i = 0; i < totalCommandArgs; i++) {
				tmp_commandArgs[i] = commandArgs[i];
			}
			tmp_commandArgs[totalCommandArgs] = NULL;	// Set Null terminator at the end of the arguments
			free(commandArgs);
			commandArgs = tmp_commandArgs;
		}
		

		// Command: pwd
		if (strcmp(commandArgs[0], "pwd") == 0) {
			char cwd[MAX_PATH];	
			getcwd(cwd, MAX_PATH);
			if (outputFile == NULL) {
				printf("%s\n", cwd);
			} else {
				outputToFile(strcat(cwd, "\n"), outputFile);
			}

		// Command: cd
		} else if (strcmp(commandArgs[0], "cd") == 0) {
			char *outStr = cd(commandArgs);
			if (outStr != NULL) {
				if (outputFile == NULL) {
					printf("%s", outStr);
				} else {
					outputToFile(outStr, outputFile);
				}
			}

		// Command: exit
		} else if (strcmp(commandArgs[0], "exit") == 0) {
			inShell = 0;	// Exit the loop
		
		// Command: other -> run an external program
		} else {
			//int runInBackground = 1;//strcmp(commandArgs[totalCommandArgs-1], "&");
			//int backgroundPID;
			int pipefd[2] = {-1, -1};
			if (totalPipedCommandArgs > 0)
				pipe(pipefd);
			// If both are non -1 then this is the flag to direct stdout to pipe
			char *outStr = runExternalProgram(commandArgs, inputFile, outputFile, pipefd);
			if (outStr != NULL) {
				printf("%s", outStr);
				// If the first command failed close the pipe if it is open
				if (totalPipedCommandArgs > 0) {
					close(pipefd[0]);
					close(pipefd[1]);
					printf("dragonshell: Pipe closed\n");
				}
			} else if (totalPipedCommandArgs > 0) {
				// If write is set to -1 but not read then this is the flag to direct pipe to stdin
				close(pipefd[1]);	// Close the write end
				pipefd[1] = -1;		// Flag the program to direct pipe to stdin
				char *outStr = runExternalProgram(pipedCommandArgs, NULL, NULL, pipefd);
				close(pipefd[0]);	// Close the read end
				if (outStr != NULL)
					printf("%s", outStr);
			}
		}
		
		
		free(input);
		free(commandArgs);
		if (totalPipedCommandArgs > 0)
			free(pipedCommandArgs);	
	}
	
	return 0;
}









// QUESTIONS FOR LAB
//
// ask if we need the -Wextra flag to be clear for full marks (NO DONT HAVE TO WORRY ABOUT IT)
//
//
// HANDLE SIGNALS: you need to handle ctrl+c and ctrl+z
//
//
// make background procesess only a total of 1
//
// do we have to check for both input and output ( ie.  <  and  >  )
