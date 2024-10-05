/*
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Name : Kelvin Creighton
# SID : 1695352
# CCID : kcreight
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h> 


#define MAX_INPUT 1024
#define MAX_PATH 1024

// Track the background process (-1 means no current background process)
int currentPID = -1;
int backgroundPID = -1;
int suspendedPID = -1;


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

void welcomeBanner() {

printf("\n"
" _    _      _                            _         ______                               _____ _          _ _ _ \n"
"| |  | |    | |                          | |        |  _  \\                             /  ___| |        | | | |\n"
"| |  | | ___| | ___ ___  _ __ ___   ___  | |_ ___   | | | |_ __ __ _  __ _  ___  _ __   \\ `--.| |__   ___| | | |\n"
"| |/\\| |/ _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ | __/ _ \\  | | | | '__/ _` |/ _` |/ _ \\| '_ \\   `--. \\ '_ \\ / _ \\ | | |\n"
"\\  /\\  /  __/ | (_| (_) | | | | | |  __/ | || (_) | | |/ /| | | (_| | (_| | (_) | | | | /\\__/ / | | |  __/ | |_|\n"
" \\/  \\/ \\___|_|\\___\\___/|_| |_| |_|\\___|  \\__\\___/  |___/ |_|  \\__,_|\\__, |\\___/|_| |_| \\____/|_| |_|\\___|_|_(_)\n"
"                                                                      __/ |                                     \n"
"                                                                     |___/                                      \n"
);

}


int runExternalProgram(char *input, char **commandArgs, char *inputFile, char *outputFile, int pipefd[2], int runInBackground) {
	pid_t pid = fork();

	// No process
	if (pid < 0) {
		perror("dragonshell");
		currentPID = -1;
		free(commandArgs);
		free(input);
		_exit(1);	
	
	// Child process
	} else if (pid == 0) {
		// Input from file to command
		if (inputFile != NULL) {
			int fd = open(inputFile, O_RDONLY);
			if (fd == -1) {
				perror("dragonshell");		
				currentPID = -1;
				free(commandArgs);
				free(input);
				_exit(1);
			}
			// Redirect fd from the input file to stdin
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		
		// Command outputs to output file
		if (outputFile != NULL) {
			int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 00644);
			if (fd == -1) {
				perror("dragonshell");
				currentPID = -1;
				free(commandArgs);
				free(input);
				_exit(1);
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
				close(pipefd[1]);
			// If the write end is -1 then direct the read end to stdin
			} else {
				dup2(pipefd[0], STDIN_FILENO);
			}
			close(pipefd[0]);
		}
		
		// Execute the external program
		if (execve(commandArgs[0], commandArgs, NULL) == -1) {
			printf("dragonshell: Command not found\n");
			currentPID = -1;
			free(commandArgs);
			free(input);
			_exit(1);
		}
		currentPID = -1;
		free(commandArgs);
		free(input);
		_exit(0);

	// Parent process
	} else if (pid > 0) {
		// Run the child process in the background
		if (runInBackground) {
			backgroundPID = pid;
			printf("PID %d is sent to background\n", pid);
		
		// If not running as background process, wait for the child process to finish
		} else {
			currentPID = pid;
			int status;
			waitpid(pid, &status, 0);
			return WIFEXITED(status) && WEXITSTATUS(status) != 0;
		}
	}
	return 0;
}

int outputToFile(char *message, char *file) {
	int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 00644);
	if (fd == -1) {
		perror("dragonshell");
		return 1;
	}

	if (write(fd, message, strlen(message)) == -1) {
		perror("dragonshell\n");
		close(fd);
		return 1;
	}
	
	close(fd);
	return 0;
}

void signal_handler(int signum) {
	printf("\n");
	if (signum == SIGTSTP && currentPID >= 0)
		suspendedPID = currentPID;
}


int main(int argc, char *argv[]) {
	welcomeBanner();

	struct rusage usage;
	getrusage(RUSAGE_CHILDREN, &usage);

	struct sigaction sa_int, sa_tstp;
	// Handle sigint
	sa_int.sa_flags = 0;
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_handler = signal_handler;
	sigaction(SIGINT, &sa_int, NULL);
	// Handle sigtstp
	sa_tstp.sa_flags = 0;
	sigemptyset(&sa_tstp.sa_mask);
	sa_tstp.sa_handler = signal_handler;
	sigaction(SIGTSTP, &sa_tstp, NULL);	

	int inShell = 1;
	while (inShell) {
		printf("dragonshell > ");
		
		// Read user command
		char *input = (char *)malloc(MAX_INPUT);
		if (fgets(input, MAX_INPUT, stdin) == NULL) {
			free(input);
			continue;
		}
			
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
		int runInBackgroundIndex = 0;	// Where the '&' appears in the tokenized array
		int i = 0;
		while (commandArgs[i] != NULL) {
			// Remove the first and last " or ' if they both exist
			int argLength = strlen(commandArgs[i]);
			if ((commandArgs[i][0] == '\"' && commandArgs[i][argLength-1] == '\"') || (commandArgs[i][0] == '\'' && commandArgs[i][argLength-1] == '\'')) {
				for (int j = 0; j < argLength-2; j++)
					commandArgs[i][j] = commandArgs[i][j+1];
				commandArgs[i][argLength-2] = '\0';
			}
			// Start counting the arguments after the '|' symbol
			if (pipeIndex > 0)
				totalPipedCommandArgs++;

			if (outputFileIndex == 0 && commandArgs[i][0] == '>') {
				outputFileIndex = i+1;
				pipeIndex = -1;		// If outputToFile exists then piping cannot exist so force it to -1 to stop it from searching for the | symbol
			}
			if (pipeIndex == 0 && commandArgs[i][0] == '|') {
				pipeIndex = i+1;
				outputFileIndex = -1;
				runInBackgroundIndex = -1;
				// The input file cannot be after the pipe so set this to -1 to stop searching for the < symbol
				if (inputFileIndex == 0)
					inputFileIndex = -1;
			}
			if (inputFileIndex == 0 && commandArgs[i][0] == '<') {
				inputFileIndex = i+1;
			}
			if (runInBackgroundIndex == 0 && commandArgs[i][0] == '&') {
				runInBackgroundIndex = i+1;
				pipeIndex = -1;
			}
			
			if (outputFileIndex == 0 && inputFileIndex == 0 && pipeIndex == 0 && runInBackgroundIndex == 0) {
				totalCommandArgs++;
			}
			i++;
		}
		// Set the end of the arguments with null terminator
		commandArgs[totalCommandArgs] = NULL;

		// Check if a command is given after the pipe character
		if (pipeIndex > 0) {
			if (totalPipedCommandArgs == 0) {
				printf("dragonshell: No pipe command given\n");
				free(commandArgs);
				free(input);
				continue;
			}
		}

		// Get the output filename, throw an error if it doesn't exist
		char *outputFile = NULL;
		if (outputFileIndex > 0) {
			if (commandArgs[outputFileIndex] == NULL) {
				printf("dragonshell: No output file given\n");
				free(commandArgs);
				free(input);
				continue;
			}
			outputFile = commandArgs[outputFileIndex];
		}	
		
		// Get the input filename, throw an error if it doesn't exist
		char *inputFile = NULL;
		if (inputFileIndex > 0) {
			if (commandArgs[inputFileIndex] == NULL) {
				printf("dragonshell: No input file given\n");
				free(commandArgs);
				free(input);
				continue;
			}
			inputFile = commandArgs[inputFileIndex];
		}

		
		// ------------
		// Command: pwd
		// ------------
		if (strcmp(commandArgs[0], "pwd") == 0) {
			char cwd[MAX_PATH];	
			getcwd(cwd, MAX_PATH);
			if (outputFile == NULL) {
				printf("%s\n", cwd);
			} else {
				outputToFile(strcat(cwd, "\n"), outputFile);
			}

		// -----------
		// Command: cd
		// -----------
		} else if (strcmp(commandArgs[0], "cd") == 0) {
			// Check if the user inputted a second argument for the new path
			if (commandArgs[1] == NULL) {
				printf("dragonshell: Expected argument to \"cd\"\n");
			} else {
				// Get the current working directory
				char cwd[MAX_PATH];
				getcwd(cwd, MAX_PATH);
				// Combine the current working directory with the users second argument as one full newPath
				char newPath[MAX_PATH*2+1];	// MAX_PATH for maximum length from cwd, MAX_PATH for maximum lnegth from commandArgs, +1 for seperator character '/' 
				char *separator = "/";
				snprintf(newPath, sizeof(newPath), "%s%s%s", cwd, separator, commandArgs[1]);	
				
				// Move into that directory and give an error if it was unsuccessful
				if (chdir(newPath) == -1) {
					perror("dragonshell");
				}
			}

		// -------------
		// Command: exit
		// -------------
		} else if (strcmp(commandArgs[0], "exit") == 0) {
			if (backgroundPID >= 0)
				kill(backgroundPID, SIGTERM); 
			if (suspendedPID >= 0) {
				kill(suspendedPID, SIGCONT);
				kill(suspendedPID, SIGTERM);
				int status;
				waitpid(suspendedPID, &status, 0);
			}
			inShell = 0;	// Exit the loop
		
		// -----------------------------------------
		// Command: other -> run an external program
		// -----------------------------------------
		} else {
			if (runInBackgroundIndex > 0 && backgroundPID != -1) {
				printf("dragonshell: one background process is already running\n");
			} else {
				int pipefd[2] = {-1, -1};
				if (totalPipedCommandArgs > 0)
					pipe(pipefd);
				// If both are non -1 then this is the flag to direct stdout to pipe
				int executeStatus = runExternalProgram(input, commandArgs, inputFile, outputFile, pipefd, runInBackgroundIndex > 0);
				
				// Run the piped command if it exists	
				if (totalPipedCommandArgs > 0) {
					// Copy the remaining command args after the pipe to the beginning of the commandArgs array
					for (int i = 0; i < totalPipedCommandArgs; i++) {
						commandArgs[i] = commandArgs[pipeIndex+i];
					}
					// Set NULL terminator at the end of the argumnets
					commandArgs[totalPipedCommandArgs] = NULL;
					close(pipefd[1]);	// Close the write end
					// If the first command executed properly then continue with the next
					if (executeStatus == 0) {
						pipefd[1] = -1;	// Flag the program to direct pipe to stdin
		/* The piped command should only have the command arguments and the pipefd. There is no input file, no outputfile, and no running in the background */
						runExternalProgram(input, commandArgs, NULL, NULL, pipefd, 0);
					} else {
						printf("dragonshell: Command not found\n");
					}
					close(pipefd[0]);	// Close the read end
				}
			}
		}
		free(commandArgs);
		free(input);
	}

	getrusage(RUSAGE_CHILDREN, &usage);
	printf("User time: %ld.%06ld seconds\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
	printf("System time: %ld.%06ld seconds\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec); 

	return 0;
}




