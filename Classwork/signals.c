#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

static void sig_action(int signum, void *siginfo, void *unused);

static void sig_install1()
{
    struct sigaction sa;

    sa.sa_sigaction = sig_action1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGUSR1, NULL, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

static void sig_install2()
{
    struct sigaction sa;

    sa.sa_sigaction = sig_action2;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGUSR2, NULL, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
    
static void sig_action1(int signum, void *siginfo, void *unused)
{
    (void) unused;
    fprintf(stderr, "User defined signal 1 \n", strsignal(signum));
}

static void sig_action2(int signum, void *siginfo, void *unused)
{
    (void) unused;
    fprintf(stderr, "User defined signal 2 \n", strsignal(signum));
}

int main(int argc, char *argv[])
{
    char c;

    sig_install1();
    sig_install2();

    if(argc > 1) {
        int id = fork();
        int i = 1;
        while(i < argc - 1) {
            if(id != 0) {
                fork();
            }
        }
    }

    if(id == 0) {
        signal(SIGUSR1, sig_action1);
    }

    if (ferror(stdin) || fflush(stdout) == EOF) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}