
/* I created a shell that can run forground and background processes, I can check the status for the background 
* processes, and I can support n number of pipes */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h> 
#include <ctype.h>
#include <signal.h>

int fork_pipe(int numcmd, char *commands[], int background);
char *trim(char *str);

void sigint_handler(int sig) {
    // You can print a new prompt or just ignore the signal
    printf("\n$ ");
    fflush(stdout);
}

int main(int argc, char **argv){
    signal(SIGINT, sigint_handler);
    int MAX_COMMAND_LENGTH;
    MAX_COMMAND_LENGTH = 50;
    int MAX_COMMANDS;
    MAX_COMMANDS = 5;
    int MAX_COMMAND_LINE_LENGTH;
    MAX_COMMAND_LINE_LENGTH = 255;
    int MAX_ARGS;
    MAX_ARGS = 50;

    char input[MAX_COMMAND_LINE_LENGTH];
    char *commands[MAX_COMMANDS];  //holds the command strings
    char *args[MAX_ARGS]; //holds the arguments for execvp

    int status;
    int i;
    int numcmd;

    while (1) {  // Start an infinite loop to keep prompting for commands
        printf("$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Break the loop if no input is provided 
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;  

        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Tokenize the input based on the pipe symbol
        int numcmd = 0;
        commands[numcmd] = strtok(input, "|");
        while (commands[numcmd] != NULL) {
            commands[numcmd] = trim(commands[numcmd]);
            numcmd++;
            commands[numcmd] = strtok(NULL, "|");
        }

        int background = 0;  // Background execution flag
        if (numcmd > 0) {
            int lastCmdIndex = strlen(commands[numcmd - 1]) - 1;
            if (lastCmdIndex >= 0 && commands[numcmd - 1][lastCmdIndex] == '&') {
                background = 1;
                commands[numcmd - 1][lastCmdIndex] = '\0';  // Remove the '&' character
                commands[numcmd - 1] = trim(commands[numcmd - 1]);  // Trim again in case of spaces before '&'
            }
        }

        if (numcmd > 0) {  // If there's at least one command, proceed to execute
            fork_pipe(numcmd, commands, background);
        }
    }

    return 0; 
}
int fork_pipe(int numcmd, char *commands[], int background) {
    int i;
    int in = 0;
    int numpipes;
    numpipes = numcmd - 1;
    

    if (numcmd == 1) {
        char *arg = strtok(commands[0], " \t");
        char *args[50];
        int argcount = 0;
        while (arg != NULL) {
            args[argcount++] = arg;
            arg = strtok(NULL, " \t");
        }
        args[argcount] = NULL;

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {  // Child process
            execvp(args[0], args);
            perror("execvp");  // execvp only returns if there is an error
            exit(EXIT_FAILURE);
        } else {
            if (!background) {
                wait(NULL);  // Wait for the child process to finish if not in background
            }
            else{
                printf("Child %d is background\n", pid);
            }
        }
        return 0;

    }
    int *fd = malloc(2 * numpipes * sizeof(int)); // dynamic allocates for fds
    
    if (fd == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Create all pipes
    for (int i = 0; i < numpipes; i++) {
        if (pipe(fd + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    in = 0;  // File descriptor for reading from the previous pipe
    for (int i = 0; i < numcmd; i++) {
        if (i < numpipes) {  // Not the last command
            int pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {  // Child process
                if (in != 0) {
                    dup2(in, 0);  // Set stdin to read from the previous pipe
                    close(in);
                }

                dup2(fd[i * 2 + 1], 1);  // Set stdout to write to the current pipe

                // Close all fds in the child
                for (int j = 0; j < 2 * numpipes; j++) {
                    close(fd[j]);
                }

                // Execute the command
                char *arg = strtok(commands[i], " \t");
                char *args[50];
                int argcount = 0;
                while (arg != NULL) {
                    args[argcount++] = arg;
                    arg = strtok(NULL, " \t");
                }
                args[argcount] = NULL;
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            }

            if (in != 0) {
                close(in);
            }

            close(fd[i * 2 + 1]);
            in = fd[i * 2];
        } else {  // Handle last command
            if (!background || i > 0) {
                if (fork() == 0) {
                    if (in != 0) {
                        dup2(in, 0);
                        close(in);
                    }

                    // Close all fds in the child
                    for (int j = 0; j < 2 * numpipes; j++) {
                        close(fd[j]);
                    }

                    char *arg = strtok(commands[i], " \t");
                    char *args[50];
                    int argcount = 0;
                    while (arg != NULL) {
                        args[argcount++] = arg;
                        arg = strtok(NULL, " \t");
                    }
                    args[argcount] = NULL;
                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    // Close all fds in the parent
    for (int i = 0; i < 2 * numpipes; i++) {
        close(fd[i]);
    }

    free(fd);

    // Wait for all child processes to complete, if not in background
    if (!background) {
        for (int i = 0; i < numcmd; i++) {
            wait(NULL);
        }
    }

    return 0;
}

char *trim(char *str) {
    char *end;

    //trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0){ 
        return str;
    }

    //trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    //write terminator character
    *(end+1) = 0;

    return str;
} 