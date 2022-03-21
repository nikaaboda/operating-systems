#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const char *url = "http://numbersapi.com/random/math?min=1&max=100&fragment&json";
extern char* fetch(char *url);
extern int parse(char *json, quiz_t *quiz);

typedef struct {
    unsigned n; /* current question number (starting at 1) */
    unsigned score; /* current total score */
    unsigned max; /* possible max score */
    char *text; /* next question (dynamically allocated) */
    int number; /* next expected correct answer */
} quiz_t;

int play(quiz_t *quiz) {
    quiz_t *newround;

    newround->n = quiz->n + 1;
    newround->max = quiz->max + 8;
    char *response = fetch(url);
    parse(response, round);

    printf("Q%d   %s\n", newround->n, newround->text);

    int ptsArr[] = { 8, 4, 2, 1};
    int answer = -1;
    int counter = 0;
    while(counter < 4 && answer != newround->number) {
        printf("%d pt> ", ptsArr[counter]);
        scanf("%d", &answer);
        if(counter != 0 && answer != newround->number) {
            if(newround->number > answer) {
                printf("Too small, try again.\n");
            } else {
                printf("Too big, try again.\n")
            }
        }
        counter++;
    }

    if(counter < 4 || answer == newround->number) {
        newround->score = quiz->score + ptsArr[counter - 1];
        printf("Congratulations your answer %d was correct!\n", answer);
    } else {
        newround->score = quiz->score;
        if(newround->number > answer) {
            printf("Too small, the correct answer was %d.\n", newround->number);
        } else {
            printf("Too big, the correct answer was %d.\n", newround->number);
        }
    }
    printf("Your total score is %d/%d points.\n", newround->score, newround->max);

    return 0;
}
