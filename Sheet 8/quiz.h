#ifndef QUIZ_H
#define QUIZ_H

typedef struct {
    unsigned n; /* current question number (starting at 1) */
    unsigned score; /* current total score */
    unsigned max; /* possible max score */
    char *text; /* next question (dynamically allocated) */
    int number; /* next expected correct answer */
} quiz_t;


extern char* fetch(char *url);
extern int parse(char *json, quiz_t *quiz);
extern int play(quiz_t *quiz);

#endif