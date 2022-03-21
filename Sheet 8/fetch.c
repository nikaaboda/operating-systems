#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

char* fetch(char *url)
{
    int fd[2];
    char *buffer;
    buffer = (char *)malloc(2000 * sizeof(char));

    pid_t forkId;

    if (pipe(fd) < 0) {
        perror("pipe");
        exit(0);
    }

    forkId = fork();

    if (forkId < 0) {      
        perror("fork");
        exit(0);
    }
    else if (forkId == 0){                                 
        close(fd[0]);

        dup2(fd[1], STDOUT_FILENO);
        execlp("curl","curl", "-s", url, NULL);
        perror("exec");
        exit(0);
    }
    else {                                                  
        close(fd[1]);

        read(fd[0], buffer, 2000);
        close(fd[0]);
    }

    return buffer;
}