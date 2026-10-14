# DragonShell

DragonShell is a small Unix-like command-line shell written in C for a systems programming course assignment. It demonstrates process creation, program execution, file descriptor manipulation, inter-process communication, signal handling, and basic shell command parsing using Unix system calls.

## Features

- **Built-in commands:** `cd`, `pwd`, and `exit`.
- **External programs:** Launch executables using `fork()` and `execve()`, with foreground execution and process waiting.
- **Background execution:** Run one external process in the background using `&`.
- **Input redirection:** Read command input from a file using `<`.
- **Output redirection:** Write command output to a file using `>`, creating or overwriting the destination.
- **Pipelines:** Pipe the standard output of one external command to another using `|`.
- **Signal handling:** Handle `SIGINT` and `SIGTSTP` through custom signal handlers.
- **Resource accounting:** Display accumulated child-process user and system CPU time when the shell exits.

## Requirements

- Linux or another Unix-like operating system
- GCC
- GNU Make

## Build and Run

Clone the repository and navigate to the project directory.

```bash
git clone <repository-url>
cd <repository-directory>
```

Compile the shell:

```bash
make
```

Run DragonShell:

```bash
./dragonshell
```

Alternatively, use the provided Makefile targets:

```bash
make run      # Build if needed and launch the shell
make remake   # Clean and rebuild
make mrun     # Clean, rebuild, and launch
make clean    # Remove generated build artifacts
```

## Usage

DragonShell displays a custom ASCII banner and presents the `dragonshell >` prompt.

### Built-in commands

```text
dragonshell > pwd
dragonshell > cd Documents
dragonshell > exit
```

`pwd` prints the current working directory. `cd` changes the working directory using a relative path. `exit` terminates the shell and performs cleanup for tracked background or suspended processes.

### External commands

External programs are executed using `execve()`. Commands should be provided using their executable paths because the implementation does not search the system `PATH`.

```text
dragonshell > /usr/bin/ls
dragonshell > /usr/bin/whoami
dragonshell > /usr/bin/date
```

### Background execution

Append `&` to run an external command in the background.

```text
dragonshell > /usr/bin/sleep 5 &
```

The shell prints the background process ID and continues accepting commands. Only one background process is supported at a time.

### Input and output redirection

Use `<` to redirect standard input from a file and `>` to redirect standard output to a file.

```text
dragonshell > /usr/bin/grep error < input.txt
dragonshell > /usr/bin/ls > output.txt
```

Output redirection creates the destination file if it does not exist and truncates it if it does.

### Piping

Use `|` to connect the output of one external command to the input of another.

```text
dragonshell > /usr/bin/find ./ | /usr/bin/sort
dragonshell > /usr/bin/grep error < input.txt | /usr/bin/wc -l
```

The implementation supports a single pipe connecting two external commands.

## Implementation

The project is implemented in C and uses Unix system calls and standard library functions to manage processes and I/O.

| Function | Purpose |
|---|---|
| `fork()` | Create a child process for an external command. |
| `execve()` | Replace the child process with the requested executable. |
| `waitpid()` | Wait for child processes and collect their exit status. |
| `open()` / `close()` | Open files and manage file descriptors. |
| `dup2()` | Redirect standard input or output. |
| `pipe()` | Establish communication between two commands. |
| `kill()` | Send signals to tracked processes during shell termination. |
| `sigaction()` | Register handlers for keyboard-related signals. |
| `getrusage()` | Collect child-process resource usage statistics. |

The shell reads commands with `fgets()`, tokenizes them by spaces, identifies supported operators, and prepares the arguments and file descriptors before executing commands.

## Testing

Testing included the assignment's provided test commands and additional commands involving `grep`, `wc`, `find`, and `sort`.

Specific tests covered:

- Built-in command behavior and working-directory changes.
- Foreground and background process execution.
- Input and output redirection.
- Piping between external commands.
- Signal handling and shell termination.
- Child-process CPU time reporting.
- Process behavior and potential zombie processes using `top`.
- Memory-related checks using Valgrind.

## Limitations

DragonShell is an educational shell implementation, not a complete replacement for Bash or another production shell. Command parsing is space-delimited, and it does not implement full shell quoting, variable expansion, wildcard expansion, or general-purpose command lookup through `PATH`. Pipeline support is limited to two external commands, and background-process management is limited.

## Project Context

This project was completed as a university systems programming assignment focused on understanding process management, file descriptors, pipes, and signals in a Unix-like environment.

**Assignment result: 97/100.** The grading feedback awarded full marks for built-in commands, external command execution, CPU-time reporting, background execution, redirection, piping, and signal handling. Deductions were related to the missing `compile` Makefile target and an incorrectly named README file.
