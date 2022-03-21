#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void signal_handler(int signum) {
    printf("Thanks for playing today.\n");
    printf("You final score is %d.\n")
    exit(signum);
}

int main() {
    quiz_t *round;
    round->n = 1;
    round->score = 0;
    round->max = 8;

    signal(SIGINT, signal_handler);

    while(true) {
        play(round);
    }

    return 0;
}