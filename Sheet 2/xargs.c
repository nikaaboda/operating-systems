#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
 

// Function to print the whole argument list
void print(char *argumentList[]) {
    int i = 0;
    while (argumentList[i] != NULL) {
        fprintf(stderr, "%s ", argumentList[i]);
        i++;
    }
}

// Function to wait for all the child processes to finish at the end of the program
void waitForAll() {
    while(true) {
        int status;
        if(waitpid(-1, &status, 0) < 0) {
            break;
        }
    }
}

// Function to execute a command on each on the batch of lines read
void batch(char *argv[], char *argumentList[], bool printStdErr, int count) {
    argumentList[count] = NULL;

    // if "-t" is specified print argument list to stderr
    if(printStdErr) {
        print(argumentList);
    }

    // if there is no command specified execute echo
    if (!argv[0]) {
        int forkId = fork();
        // error handling for fork
        if(forkId == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (forkId == 0) {
            execvp("echo", argumentList - 1);
            perror("execvp");
            exit(1);
        }

        // wait for child to finish (blocking call)
        int status;
        waitpid(-1, &status, 0);
    } else { // execute the command that's specified
        int forkId = fork();
        // error handling for fork
        if(forkId == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(forkId == 0) { 
            execvp(argv[0], argumentList - 1);
            perror("execvp");
            exit(1);
        }

        // wait for child and handle errors (blocking call)
        int status;
        if(waitpid(-1, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        };

    }
}

// Function to handle the number of processes running simultaniosly
void controlForks(int activeProcesses[], int limitOfProcesses, char *argv[],
                 char *argumentList[], bool printStdErr, int count) {
    bool limitReached = true;
    // determine if array of running processes is full
    for(int k = 0; k < limitOfProcesses - 1; k++) {
        if(activeProcesses[k] == 0) {
            limitReached = false;
        }
    }

    // if max number of processes are running wait for one of them to finish
    if (limitReached) {
        int status;
        if(waitpid(-1, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        };
    }

    // error handling for fork
    int forkId = fork();
    if(forkId == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // execute new command on read lines in new child process
    if(forkId == 0) {
        batch(argv, argumentList, printStdErr, count);
        exit(1);
    }

    // save the id of new process in array's empty slot 
    // or slot where terminated process's id is
    for(int k = 0; k < limitOfProcesses - 1; k++) {
        int waitId;
        if(activeProcesses[k] == 0 || waitpid(activeProcesses[k], &waitId, WNOHANG) == activeProcesses[k]) {
            activeProcesses[k] = forkId;
            break;
        }
    }
}
 
int main(int argc, char *argv[]) {
    int limit = 128;
    char *line_buf = NULL;
    size_t line_buf_size = 0;
    char opt;
    int limitOfProcesses = 1;
    bool printStdErr = false;
    
    // Determine commands that were specified in command line
    while((opt = getopt(argc, argv, "n:tj:")) != -1) 
    { 
        if(opt == 'n') {
            limit = atoi(optarg);
        }
 
        if(opt == 't') {
            printStdErr = true;
        }
 
        if(opt == 'j') {
            limitOfProcesses = atoi(optarg);
        }
    }
    
    // move pointer to first command
    argc -= optind;
    argv += optind;
    
    char *argumentList[limit + 1];

    // Create an array for ids of running child processes 
    // and initialize it with zeros
    int activeProcesses[limitOfProcesses - 1];
    for(int k = 0; k < limitOfProcesses - 1; k++) {
        activeProcesses[k] = 0;
    } 
 
    int count = 0;
    ssize_t nread;
    // read from input until EOF
    while((nread = getline(&line_buf, &line_buf_size, stdin)) != -1) {
        // create a copy of the line read to save it
        char *lineCopy = strndup(line_buf, nread - 1);
        // handle errors for strndup
        if(lineCopy == NULL) {
            perror("strndup");
            exit(EXIT_FAILURE);
        }
        // save line to arguments list
        argumentList[count] = lineCopy;
        count++;

        // if program read max number of lines 
        // execute command on them in child process
        if (count == limit) {
            controlForks(activeProcesses, limitOfProcesses, argv,
                 argumentList, printStdErr, count);
            
            count = 0;
        }
    }

    // if less than the max number of lines were read
    // but there's no more to read, execute command on what was read
    if(count > 0) {
        controlForks(activeProcesses, limitOfProcesses, argv,
                 argumentList, printStdErr, count);
    }

    //wait for all child processes to finish and return
    waitForAll();
    
    return 0;
}